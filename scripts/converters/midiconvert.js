var fs = require("fs");
const { off } = require("process");
var parseMidi = require("midi-file").parseMidi;

var filename = process.argv[2];
var baseName = filename.split(".").slice(0,-1).join(".");
var outputFilename = process.argv[3] || (baseName + ".gtm2");

var inputFile = fs.readFileSync(filename);
var parsedInput = parseMidi(inputFile);

var microsecondsPerFrame = 16666.666667;

console.log(parsedInput.header);

var numTracks = parsedInput.header.numTracks;
var totalEntries = 0;

var mergedTracks = [];
var tempo = 120; //microseconds per quarter note

parsedInput.tracks.forEach((track, i) => {
    var absTime = 0;

    track.forEach((trackEvent) => {
        if(trackEvent.type == "setTempo") {
            tempo = 60000000 / trackEvent.microsecondsPerBeat;
        }

        absTime += trackEvent.deltaTime * 60000000 / (tempo * parsedInput.header.ticksPerBeat);
        trackEvent.absTime = absTime;
        trackEvent.absFrames = Math.floor(absTime / microsecondsPerFrame);
        trackEvent.trackNum = i;
    });
    mergedTracks = mergedTracks.concat(track);
});

mergedTracks.sort((a,b) => a.absFrames - b.absFrames);

var lastAbsTime = mergedTracks[0].absFrames;
mergedTracks.forEach((event) => {
    event.deltaTime = event.absFrames - lastAbsTime;
    lastAbsTime = event.absFrames;
});

var timeBucketedTracks = [];
var currentBucket = [];
lastAbsTime = 0;

mergedTracks.forEach((event) => {
    if(event.absFrames > lastAbsTime) {
        timeBucketedTracks.push(currentBucket);
        currentBucket = [];
        lastAbsTime = event.absFrames;
    }
    currentBucket.push(event);
});

if(currentBucket.length > 0) {
    timeBucketedTracks.push(currentBucket);
}

timeBucketedTracks.forEach((bucket) => {
    bucket.sort((a,b) => a.absTime - b.absTime);
});

var frameBuffers = timeBucketedTracks.map((bucket) => {
    var frame = {};
    var deltaTime = 0;
    bucket.forEach((event) => {
        if(event.deltaTime > 0) {
            deltaTime = event.deltaTime;
        }
        if(event.type == "noteOn" || event.type == "noteOff") {
            frame[event.channel] = {
                note : (event.type == "noteOn") ? event.noteNumber : 0,
                type : event.type,
                channel : event.channel
            }
        }
    });
    var noteCount = 0;
    var noteMask = 0;
    var tracks = [];
    for(var i in frame)  {
        noteCount++;
        noteMask |= (1 << frame[i].channel);
        tracks.push(i);
    }
    tracks.sort();
    var frameBuf = Buffer.alloc(noteCount+2);
    var offset = 0;
    frameBuf.writeUint8(deltaTime, offset++);
    frameBuf.writeUint8(noteMask, offset++);
    tracks.forEach((n) => {
        frameBuf.writeUint8(frame[n].note, offset++);
    });
    return frameBuf;
});

var outBuf = Buffer.concat(frameBuffers);
var zeroBuf = Buffer.alloc(1);

var stream = fs.createWriteStream(outputFilename);
stream.on("error", console.error);
stream.write(outBuf);
stream.write(zeroBuf);
stream.end();
