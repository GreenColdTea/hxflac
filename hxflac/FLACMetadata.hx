package hxflac;

class FLACMetadata {
    public var title:String;
    public var artist:String;
    public var album:String;
    public var genre:String;
    public var year:String;
    public var track:String;
    public var comment:String;
    
    public function new() {}
    
    public function toString():String {
        return 'FLACMetadata [title: $title, artist: $artist, album: $album, genre: $genre, year: $year, track: $track]';
    }
}