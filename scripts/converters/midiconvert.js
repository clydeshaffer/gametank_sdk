const fs = require("fs");
const parseMidi = require("midi-file").parseMidi;
const getAssetConfig = require("../common/assetConfig").getAssetConfig;
const path = require('path');

const SongFlags = {
    useVelocity : 1,
    useProgChanges : 2,
    reserved2 : 4,
    reserved3 : 8,
    reserved4 : 16,
    reserved5 : 32,
    reserved6 : 64,
    reserved7 : 128,
};

var filename = process.argv[2];
var baseName = filename.split(".").slice(0,-1).join(".");
var outputFilename = process.argv[3] || (baseName + ".gtm2");

var inputFile = fs.readFileSync(filename);
var parsedInput = parseMidi(inputFile);

const midiFileConfig = getAssetConfig(filename);

if(midiFileConfig) {
    console.log("Found config for " + path.basename(filename));
    console.log(midiFileConfig);
}

const useVelocity = midiFileConfig ? midiFileConfig.useVelocity : false;
var instrumentSettings = [0, 3, 4, 0]; //piano, slapbass, snare, piano
if(Array.isArray(midiFileConfig?.instruments)) {
    instrumentSettings = midiFileConfig.instruments;
}

var bytesPerNote = 1;
if(useVelocity) bytesPerNote++;

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
                channel : event.channel,
                velocity : event.velocity
            }
        }
    });
    var noteCount = 0;
    var noteMask = 0;
    var tracks = [];
    for(var i in frame)  {
        if(frame[i].channel > 3) continue;
        noteCount++;
        noteMask |= (1 << frame[i].channel);
        tracks.push(i);
    }
    tracks.sort();
    var extraTime = 0;
    if(deltaTime > 255) {
        extraTime = deltaTime - 255;
        deltaTime = 255;
    }
    var frameBuf = Buffer.alloc((bytesPerNote * noteCount)+2+(2 * Math.ceil(extraTime / 128)));
    var offset = 0;
    while(extraTime > 0) {
        frameBuf.writeUint8(Math.min(128, extraTime), offset++);
        frameBuf.writeUint8(0, offset++);
        extraTime -= 128;
    }
    frameBuf.writeUint8(deltaTime, offset++);
    frameBuf.writeUint8(noteMask, offset++);
    tracks.forEach((n) => {
        frameBuf.writeUint8(frame[n].note, offset++);
        if(useVelocity) {
            frameBuf.writeUint8(Math.round(frame[n].velocity * (6 / 8)), offset++);
        }
    });
    return frameBuf;
});

var outBuf = Buffer.concat(frameBuffers);
var zeroBuf = Buffer.alloc(1);

var songConfigHeader = 0;

if(useVelocity) {
    songConfigHeader |= SongFlags.useVelocity;
}

const songConfigBuf = Buffer.alloc(1);
songConfigBuf.writeUint8(songConfigHeader);

const instrumentSettingsBuf = Buffer.alloc(4);
instrumentSettingsBuf.writeUint8(instrumentSettings[0], 0);
instrumentSettingsBuf.writeUint8(instrumentSettings[1], 1);
instrumentSettingsBuf.writeUint8(instrumentSettings[2], 2);
instrumentSettingsBuf.writeUint8(instrumentSettings[3], 3);

var stream = fs.createWriteStream(outputFilename);
stream.on("error", console.error);
stream.write(songConfigBuf);
stream.write(instrumentSettingsBuf);
stream.write(outBuf);
stream.write(zeroBuf);
stream.end();
