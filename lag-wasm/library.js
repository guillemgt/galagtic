mergeInto(LibraryManager.library, {
    setClipboard: function(text){
        var textArea = document.createElement("textarea");

        //
        // *** This styling is an extra step which is likely not required. ***
        //
        // Why is it here? To ensure:
        // 1. the element is able to have focus and selection.
        // 2. if element was to flash render it has minimal visual impact.
        // 3. less flakyness with selection and copying which **might** occur if
        //    the textarea element is not visible.
        //
        // The likelihood is the element won't even render, not even a flash,
        // so some of these are just precautions. However in IE the element
        // is visible whilst the popup box asking the user for permission for
        // the web page to copy to the clipboard.
        //

        // Place in top-left corner of screen regardless of scroll position.
        textArea.style.position = 'fixed';
        textArea.style.top = 0;
        textArea.style.left = 0;

        // Ensure it has a small width and height. Setting to 1px / 1em
        // doesn't work as this gives a negative w/h on some browsers.
        textArea.style.width = '2em';
        textArea.style.height = '2em';

        // We don't need padding, reducing the size if it does flash render.
        textArea.style.padding = 0;

        // Clean up any borders.
        textArea.style.border = 'none';
        textArea.style.outline = 'none';
        textArea.style.boxShadow = 'none';

        // Avoid flash of white box if rendered for any reason.
        textArea.style.background = 'transparent';


        textArea.value = text;

        document.body.appendChild(textArea);

        textArea.select();

        try {
            var successful = document.execCommand('copy');
            if(!successful)
            console.log("Couldn't copy text");
        } catch (err) {
            console.log('There was an error while copying the text');
        }

        document.body.removeChild(textArea);
    },
    setCookie: function(p_name, p_value){
        let canAccessLocalStorage = true; // We have to do this, as otherwise wasm + itch.io + (blocking 3rd party cookies) give an error
        try {
            if(localStorage == undefined && window.localStorage != undefined)
                localStorage = window.localStorage;
            if(localStorage && localStorage["a"]){}
        } catch(e) {
            canAccessLocalStorage = false;
        }


        var name = UTF8ToString(p_name);
        var value = UTF8ToString(p_value);
        if(canAccessLocalStorage && localStorage){
            localStorage[name] = value;
        }else{
            var date = new Date();
            date.setTime(date.getTime() + (365*24*60*60*1000));
            document.cookie = name + "=" + value + "; expires=" + date.toUTCString();
        }
    },
    getCookie: function(p_name){
        let canAccessLocalStorage = true; // We have to do this, as otherwise wasm + itch.io + (blocking 3rd party cookies) give an error
        try {
            if(localStorage == undefined && window.localStorage != undefined)
                localStorage = window.localStorage;
            if(localStorage && localStorage["a"]){}
        } catch(e) {
            console.log("Please, enable 3rd party cookies on this site (so that we can save your progress).");
            document.getElementById("third-party-cookies-alert").style.display = "block";
            canAccessLocalStorage = false;
        }

        var name = UTF8ToString(p_name);
        if(canAccessLocalStorage && localStorage){
            if(localStorage[name])
                return allocate(intArrayFromString(localStorage[name]), 'i8', ALLOC_NORMAL);
        }else{
            var ca = document.cookie.split(";");
            for(var i=0; i<ca.length; i++){
                var c = ca[i];
                while (c.charAt(0)==' ')
                    c = c.substring(1);
                if (c.indexOf(name+"=") == 0){
                    var value = c.substring(name.length+1,c.length);
                    return allocate(intArrayFromString(value), 'i8', ALLOC_NORMAL);
                }
            }
        }

        return null;
    },
    loadAudio: function(slot, path_c, volume){
        if(window.audioCtx == undefined){
            window.audioCtx = new (window.AudioContext || window.webkitAudioContext)();
        }
        if(window.audios == undefined){
            window.audios = [];
            window.audioVolumes = [];
            window.audioIsLooped = [];
            window.audioSources = [];
        }

        var path = UTF8ToString(path_c);

        window.audioVolumes[slot] = volume;
        window.audioIsLooped[slot] = false;

        var request = new XMLHttpRequest();
        request.open('GET', path, true);
        request.responseType = 'arraybuffer';

        request.onload = function() {
            window.audioCtx.decodeAudioData(request.response, function(buffer) {
                window.audios[slot] = buffer;
            });
        }
        request.send();
    },
    loadAudioLoop: function(slot, path_c, volume){
        if(window.audioCtx == undefined){
            window.audioCtx = new (window.AudioContext || window.webkitAudioContext)();
        }
        if(window.audios == undefined){
            window.audios = [];
            window.audioVolumes = [];
            window.audioIsLooped = [];
            window.audioSources = [];
        }

        var path = UTF8ToString(path_c);

        window.audioVolumes[slot] = volume;
        window.audioIsLooped[slot] = true;

        var request = new XMLHttpRequest();
        request.open('GET', path, true);
        request.responseType = 'arraybuffer';

        request.onload = function() {
            window.audioCtx.decodeAudioData(request.response, function(buffer) {
                window.audios[slot] = buffer;
            });
        }
        request.send();
    },
    playAudio: function(slot){
        var source = window.audioCtx.createBufferSource();

        source.buffer = window.audios[slot];

        var gainNode = window.audioCtx.createGain();
        source.connect(gainNode);
        gainNode.connect(window.audioCtx.destination);
        gainNode.gain.value = window.audioVolumes[slot];

        source.loop = window.audioIsLooped[slot];
        source.start(0);

        if (!source.stop)
            source.stop = source.noteOff;

        source.onended = function(){
            let index = -1;
            for(let i=0; i<window.audioSources.length; i++){
                if(window.audioSources[i][0] == this){
                    index = i;
                    break;
                }
            }
            window.audioSources.splice(index, 1);
            this.disconnect();
        }

        window.audioSources.push([source, slot, gainNode]);
    },
    stopAudio: function(slot){
        for(let i=window.audioSources.length-1; i>=0; i--){
            if(window.audioSources[i][1] == slot){
                window.audioSources[i][0].stop(0);
                window.audioSources[i][0].disconnect();
                window.audioSources.splice(i, 1);
            }
        }
    },
    doEndingAudioEffect: function(){
        let pitchUpInterval = setInterval(function(){
            for(let i=window.audioSources.length-1; i>=0; i--){
                if(window.audioSources[i][1] == 10){
                    window.audioSources[i][0].detune.value += 300;
                }
            }
        }, 100);
        setTimeout(function(){
            _playAudio(11);
        }, 2500);
        setTimeout(function(){
            clearInterval(pitchUpInterval);
            let volumeDownInterval = setInterval(function(){
                let m = 1;
                for(let i=window.audioSources.length-1; i>=0; i--){
                    if(window.audioSources[i][1] == 10){
                        window.audioSources[i][2].gain.value -= 0.01;
                        m = Math.min(m, window.audioSources[i][2].gain.value);
                    }
                }
                if(m < 0){
                    _stopAudio(10);
                    clearInterval(volumeDownInterval);
                }
            }, 10);
        }, 3700);
    }
});
