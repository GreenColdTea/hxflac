package hxflac;

#if cpp
@:buildXml("<include name='${haxelib:hxflac}/build.xml' />")
@:include("hxflac.hpp")
@:keep
@:allow(hxflac.FLACHelper)
extern class FLAC {
    @:native("hxflac_get_version_string")
    static function _getVersionString():cpp.ConstCharStar;
    
    @:native("hxflac_to_bytes")
    static function _toBytes(data:cpp.ConstPointer<cpp.UInt8>, length:Int, resultData:cpp.RawPointer<cpp.RawPointer<cpp.UInt8>>, resultLength:cpp.RawPointer<Int>,
        sampleRate:cpp.RawPointer<Int>,
        channels:cpp.RawPointer<Int>,
        bitsPerSample:cpp.RawPointer<Int>
    ):Void;
    
    @:native("hxflac_get_metadata")
    static function _getMetadata(data:cpp.ConstPointer<cpp.UInt8>, length:Int, title:cpp.RawPointer<cpp.ConstCharStar>, artist:cpp.RawPointer<cpp.ConstCharStar>,
        album:cpp.RawPointer<cpp.ConstCharStar>,
        genre:cpp.RawPointer<cpp.ConstCharStar>,
        year:cpp.RawPointer<cpp.ConstCharStar>,
        track:cpp.RawPointer<cpp.ConstCharStar>,
        comment:cpp.RawPointer<cpp.ConstCharStar>
    ):Bool;
    
    @:native("hxflac_free_string")
    static function _freeString(str:cpp.ConstCharStar):Void;
}
#else
#error "FLAC is only supported on the cpp target."
#end