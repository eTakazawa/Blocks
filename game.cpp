#define DISALLOW_COPY_AND_ASSIGN(TypeName) TypeName(const TypeName&);void operator=(const TypeName&)

#include <fstream>
#include <iostream>
#include <vector>
#include <set>

using namespace std;

class GameInfo {
public:
  static const int kNumInitPlayerBlocks = 21;
  static const int kMaxTileLength = 5;
  static const int kBoardHeight = 14;
  static const int kBoardWidth = 14;
};
class Tile {
public:
  // Constructer
  Tile(int type, int height, int width, vector<string> tile) {
    type_ = type;
    height_ = height;
    width_ = width;
    tile_ = tile;
  }
  Tile(const Tile& tile) {
    type_ = tile.type();
    height_ = tile.height();
    width_ = tile.width();
    tile_ = tile.tile();
  }
  // Methods
  bool Get(int x, int y) const {
    if(IsOut(x, y)) cerr << "Out of tile's index" << endl;
    return tile_[y][x] == 'o';
  }
  bool IsOut(int x, int y) const {
    return x < 0 || y < 0 || x >= width_ || y >= height_;
  }
  void rotate() {
    vector<string> rotated(GameInfo::kMaxTileLength, string(GameInfo::kMaxTileLength, '.'));
    int l, r, u, d;
    l = u = GameInfo::kMaxTileLength;
    d = r = -1;
    for (int y = 0; y < GameInfo::kMaxTileLength; y++) {
      for (int x = 0; x < GameInfo::kMaxTileLength; x++) {
        if (x >= height_ || (GameInfo::kMaxTileLength-1)-y >= width_) continue;
        rotated[y][x] = tile_[x][(GameInfo::kMaxTileLength-1) - y];
        if (rotated[y][x] == 'o') {
          l = min(l,x);
          r = max(r,x);
          u = min(u,y);
          d = max(d,y);
        }
      }
    }
    tile_ = packed(l, r, u, d, rotated);
    width_ = r - l + 1;
    height_ = d - u + 1;
  }
  void Show() {
    cerr << height_ << " " << width_ << endl;
    for (int h = 0; h < height_; h++){
      cerr << tile_[h] << endl;
    }
  }
  Tile Clone() {
    return Tile( type_, height_, width_, tile_ );
  }
  // Getter
  vector<string> tile() const { return tile_; }
  int type() const { return type_; }
  int height() const { return height_; }
  int width() const { return width_; }
private:
  vector<string> packed(int l,int r,int u,int d,const vector<string>& rotated) {
    vector<string> new_tile;
    for (int y = u; y <= d; y++) {
      new_tile.push_back(rotated[y].substr(l,r-l+1));
    }
    return new_tile;
  }
  int type_;
  int height_,width_;
  vector<string> tile_;
};
class TileGenerator {
public:
  // Constructer
  TileGenerator(string filename): filename_(filename) {
    ifstream ifs(filename);
    if (ifs.fail()) {
      cerr << "file read failed" << endl;
    }
    int num_types;
    ifs >> num_types;
    num_types_ = num_types;
    for (int i = 0; i < num_types; i++) {
      int type,height,width;
      ifs >> type >> height >> width;
      vector<string> raw_tile(height);
      for (int h=0; h < height; h++){
        ifs >> raw_tile[h];
      }
      tile_db_.push_back(Tile(type, height, width, raw_tile));
    }
  }
  TileGenerator(const TileGenerator& tile_generator):
    num_types_(tile_generator.num_types()),
    filename_(tile_generator.filename()),
    tile_db_(tile_generator.tile_db()){}
  // Methods
  Tile Generate(int type) {
    return tile_db_[type].Clone();
  }
  void Show() {
    cerr << "filename =" << filename_ << endl;
    cerr << "num_types=" << num_types_ << endl;
    for (int i = 0; i < num_types_; i++) {
      cerr << "ID=" << i << " H,W=";
      tile_db_[i].Show();
    }
  }
  // Getter
  int num_types() const { return num_types_; }
  string filename() const { return filename_; }
  vector<Tile> tile_db() const { return tile_db_; }
private:
  // TileGenerator &operator=(const TileGenerator &o);
  int num_types_;
  string filename_;
  vector<Tile> tile_db_;
};
class Block {
public:
  // Constructer
  Block(int id, int owner, const Tile& tile): id_(id),owner_(owner),tile_(tile) {
    if (owner == 0) cerr << "\"0\" is unavailable for Owner ID" << endl;
  }
  void Show() {
    cerr << "id=" << id_ << " owner=" << owner_ << endl;
    tile_.Show();
  }
  // Getter
  bool GetTile(int x, int y) const { return tile_.Get(x, y); }
  int GetHeight() const { return tile_.height(); }
  int GetWidth() const { return tile_.width(); }
  int id() const { return id_; }
  int owner() const { return owner_; }
  Tile tile() const { return tile_; }
  // Setter
  void set_owner(int owner) { owner_ = owner; }
  void set_id(int id) { id_ = id; }
private:
  int id_, owner_;
  Tile tile_;
};
class BlockGenerator {
public:
  // Constructer
  BlockGenerator(TileGenerator tile_generator): id_(0),tile_generator_(tile_generator) {}
  BlockGenerator(const BlockGenerator& block_generator) :
                 id_(block_generator.GetLastID()),
                 tile_generator_(block_generator.tile_generator()){}
  // Methods
  Block Generate(int type, int owner) {
    id_++;
    return Block( id_, owner, tile_generator_.Generate(type));
  }
  TileGenerator tile_generator() const { return tile_generator_; }
  int GetLastID() const { return id_; }
  void Show() {
    cerr <<"Last ID=" << GetLastID() << endl;
    tile_generator_.Show();
  }

  int num_types() {
    return tile_generator_.num_types();
  }
private:
  // BlockGenerator &operator=(const BlockGenerator &o);
  int id_;
  TileGenerator tile_generator_;
};
class Board {
public:
  // Constructer
  Board(int height, int width): height_(height), width_(width),
                                board_(height,vector<int>(width)) { }
  // Methods
  int Get(int x, int y) const {
    if (IsOut(x, y)){
      cerr << "Out of board's index" << endl;
      return -1;
    }
    return board_[y][x];
  }
  bool IsValidPutBlock(const Block& block, int put_x, int put_y) {
    for (int h = 0; h < block.GetHeight(); h++) {
      for (int w = 0; w < block.GetWidth(); w++) {
        int x = put_x + w;
        int y = put_y + h;
        if (IsOut(x, y)) return false;
        if (!block.GetTile(w, h)) continue;
        if (board_[y][x] != EMPTY) return false;
      }
    }
    return true;
  }
  bool IsOut(const int& x, const int& y) const {
    return x < 0 || y < 0 || x >= width_ || y >= height_;
  }
  bool PutBlock(const Block& block, int put_x, int put_y) {
    if (!IsValidPutBlock(block, put_x, put_y)) return false;
    for (int h = 0; h < block.GetHeight(); h++) {
      for (int w = 0; w < block.GetWidth(); w++) {
        int x = put_x + w;
        int y = put_y + h;
        if (block.GetTile(w, h)) board_[y][x] = block.owner();
      }
    }
    return true;
  }
  void Show() {
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
        cerr << board_[y][x] << " ";
      }
      cerr << endl;
    }
  }
  // Getter
  int height() const { return height_; }
  int width() const { return width_; }
private:
  enum Type {
    EMPTY = 0,
  };
  int height_,width_;
  vector<vector<int>> board_;
};
class Player {
public:
  // Constructer
  Player(int id): id_(id) {
    if (id == 0) cerr << "\"0\" is unavailable for Player ID" << endl;
  }
  void InitBlocks(BlockGenerator& block_generator) {
    num_blocks_ = block_generator.num_types();
    for(int i = 0; i < num_blocks_; i++) {
      blocks_.push_back(block_generator.Generate(i, id_));
      unused_blockids.insert(i);
    }
  }
  bool PutBlock(Board& board, int blockid, int put_x, int put_y) {
    return board.PutBlock(blocks_[blockid], put_x, put_y);
  }
  // Methods
  void Show() {
    cerr << "Player " << id_ << " " << endl;
    cerr << "NumOfBlocks " << num_blocks_ << endl;
    // for (int i = 0; i < num_blocks_; i++) {
    //   blocks_[i].Show();
    // }
  }
  // Getter
  int id() const { return id_; }
  int num_blocks() const { return num_blocks_; }
private:
  int id_,num_blocks_;
  vector<Block> blocks_;
  set<int> unused_blockids;
};
class Game {
public:
  Game(const BlockGenerator& block_generator):
          num_players_(0),
          board_(GameInfo::kBoardHeight, GameInfo::kBoardWidth),
          block_generator_(block_generator){}
  int AddPlayer(const Player& player) {
    players_.push_back(player);
    num_players_++;
    return num_players_;
  }
  bool IsEnd() {
    
  }

  void Show() {
    block_generator_.Show();
    board_.Show();
  }
  void BoardShow() {
    board_.Show();
  }
  void PlayersShow() {
    for (int i = 0; i < num_players_; i++) {
      players_[i].Show();
    }
  }
  Board* pboard() { return &board_; }
  Board board() { return board_; }
  int num_players() const { return num_players_; }
  Player player(int id) const { return players_[id]; }
  vector<Player> players() const { return players_; }
private:
  int num_players_;
  Board board_;
  vector<Player> players_;
  BlockGenerator block_generator_;
};
class GameManager {
public:
  GameManager(Game game): game_(game) {}
  void Loop() {
    for (Player player : game_.players()) {
      player.Show();
      game_.BoardShow();
      while (true) {
        cerr << "Input Type PutX PutY" << endl;
        int type,put_x,put_y;
        cin >> type >> put_x >> put_y;
        if(player.PutBlock(*(game_.pboard()),type,put_x,put_y))break;
      }
    }
    game_.BoardShow();
  }
private:
  Game game_;
};

int main(void){
  TileGenerator tile_generator("tile_pattern.txt");
  BlockGenerator block_generator(tile_generator);

  // BlockGenerator Test
  // vector<Block> test_blocks;
  // for (int i = 0; i < GameInfo::kNumInitPlayerBlocks; i++) {
  //   test_blocks.push_back(block_generator.Generate(i,i%4+1));
  // }
  // test_blocks[18].Show();

  // Board board(14, 14);
  
  // player1.PutBlock(board, 12, 4, 5);
  // player1.Show();
  // board.Show();

  // TileGenerator tile_generator("tile_pattern.txt");
  // Tile Rotate Test
  // for(int i=0;i<GameInfo::kNumInitPlayerBlocks;i++) {
  //   Tile tile = tile_generator.Generate(i);
  //   for(int t=0;t<4;t++){
  //     tile.Show();
  //     tile.rotate();
  //     cerr << endl;
  //   }
  // }

  Game game(block_generator);
  for (int i = 1; i <= 2; i++) {
    Player player(i);
    player.InitBlocks(block_generator);
    game.AddPlayer(player);
  }
  GameManager game_manager(game);
  game_manager.Loop();

  // game.Show();
  // game.PlayersShow();
  return 0;
}


