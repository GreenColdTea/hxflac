package hxflac.flixel;

import flixel.FlxG;
import flixel.sound.FlxSound;
import haxe.io.Bytes;
import hxflac.openfl.FLACStreamedSound;
import openfl.events.Event;

class FlxStreamedSound extends FlxSound
{
    var _stream:FLACStreamedSound;
    var _bytes:Bytes;
    var _chunkSize:Int = 65536;

    public function new(?bytes:Bytes, looped:Bool = false, chunkSize:Int = 65536)
    {
        super();

        this.looped = looped;
        _chunkSize = chunkSize;

        if (bytes != null)
            loadStreamedFlac(bytes, looped, false, null, chunkSize);
    }

    public function loadStreamedFlac(bytes:Bytes, looped:Bool = false, autoDestroy:Bool = false, ?onComplete:()->Void, chunkSize:Int = 65536):FlxStreamedSound
    {
        if (bytes == null)
        {
            FlxG.log.error("Expected FLAC bytes, got null");
            return this;
        }

        cleanup(true);

        _bytes = bytes;
        _chunkSize = chunkSize;
        _stream = new FLACStreamedSound(_bytes, _chunkSize);

        this.looped = looped;
        this.autoDestroy = autoDestroy;
        this.onComplete = onComplete;

        _length = _stream.length * 1000;
        endTime = _length;
        exists = true;
        active = false;
        _paused = false;
        _time = 0;

        updateTransform();
        return this;
    }

    public var paused(get, never):Bool;
    inline function get_paused():Bool return _paused;

    override public function play(forceRestart = false, startTime = 0.0, ?endTime:Float):FlxSound
    {
        if (!exists || _stream == null)
            return this;

        if (forceRestart)
        {
            restart(startTime);
            this.endTime = endTime;
            return this;
        }

        if (_channel != null)
            return this;

        if (_paused)
            resume();
        else
        {
            #if (flixel >= "6.2.0")
            loopCount = 0;
            #end
            startSound(startTime);
        }

        this.endTime = endTime;
        return this;
    }

    public function restart(?startTime:Float = 0):FlxStreamedSound
    {
        if (_stream == null)
            return this;

        if (_channel != null)
        {
            _channel.removeEventListener(Event.SOUND_COMPLETE, stopped);
            _channel.stop();
            _channel = null;
        }

        _paused = false;
        _time = startTime;
        #if (flixel >= "6.2.0")
        loopCount = 0;
        #end
        startSound(startTime);

        return this;
    }

    override public function resume():FlxSound
    {
        if (_paused && _stream != null)
            startSound(_time);

        return this;
    }

    override public function pause():FlxSound
    {
        if (_channel == null || _stream == null)
            return this;

        _time = _stream.playbackTime * 1000;
        _paused = true;
        cleanup(false, false);
        return this;
    }

    override public function update(elapsed:Float):Void
    {
        if (_channel == null || _stream == null || _paused)
            return;

        _time = _stream.playbackTime * 1000;

        #if (flixel >= "6.2.0")
        updateProximity();
        #end
        updateTransform();

        if (endTime != null && _time >= endTime)
            stopped();
    }

    override function updateTransform():Void
    {
        if (_transform == null)
            return;

        final vol = calcTransformVolume();

        if (_stream != null)
            _stream.volume = vol;

        if (_channel != null)
            _channel.soundTransform = _transform;

        amplitudeLeft = 0;
        amplitudeRight = 0;
        amplitude = 0;
    }

    override function startSound(StartTime:Float):Void
    {
        if (_stream == null)
            return;

        _time = StartTime;
        _paused = false;

        _stream.seekMilliseconds(StartTime);
        _channel = _stream.play(0, 0, _transform);

        if (_channel != null)
        {
            _channel.addEventListener(Event.SOUND_COMPLETE, stopped);
            active = true;
            updateTransform();
        }
        else
        {
            exists = false;
            active = false;
        }
    }

    override function stopped(?_):Void
    {
        if (onComplete != null)
            onComplete();

        if (looped #if (flixel >= "6.2.0") && (loopUntil == -1 || loopCount < loopUntil) #end)
        {
            #if (flixel >= "6.2.0")
            loopCount++;
            #end
            cleanup(false, false);
            startSound(loopTime);
        }
        else
        {
            _time = 0;
            cleanup(autoDestroy, false);
        }
    }

    override function cleanup(destroySound:Bool, resetPosition:Bool = true):Void
    {
        if (_channel != null)
        {
            _channel.removeEventListener(Event.SOUND_COMPLETE, stopped);
            _channel.stop();
            _channel = null;
        }

        active = false;

        if (destroySound)
        {
            if (_stream != null)
            {
                _stream.close();
                _stream = null;
            }

            _bytes = null;
            exists = false;
        }

        if (resetPosition)
        {
            _time = 0;
            _paused = false;
            #if (flixel >= "6.2.0")
            loopCount = 0;
            #end
        }
    }

    override function onFocus():Void
    {
        if (_resumeOnFocus)
        {
            _resumeOnFocus = false;
            resume();
        }
    }

    override function onFocusLost():Void
    {
        _resumeOnFocus = !_paused && _channel != null;
        pause();
    }

    override public function destroy():Void
    {
        if (_channel != null)
        {
            _channel.removeEventListener(Event.SOUND_COMPLETE, stopped);
            _channel.stop();
            _channel = null;
        }

        if (_stream != null)
        {
            _stream.close();
            _stream = null;
        }

        _bytes = null;

        super.destroy();
    }

    public function seek(time:Float):FlxStreamedSound
    {
        if (_stream == null)
        {
            _time = time;
            return this;
        }

        if (_channel != null && !_paused)
        {
            _channel.removeEventListener(Event.SOUND_COMPLETE, stopped);
            _channel.stop();
            _channel = null;
            startSound(time);
        }
        else
        {
            _time = time;
        }

        return this;
    }
}