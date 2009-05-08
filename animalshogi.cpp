#include <iostream>
#include <cassert>
#include <map>
#include <set>
#include <vector>
#include <string>

#define BUFFER_SIZE (1000 * 1000)

typedef char koma_v;

union pos {
  struct {
    koma_v x;
    koma_v y;
  };
  short value;
};
struct pos_comparer {
  bool operator()(const pos& x, const pos& y) const {
    return x.value < y.value;
  }
};
typedef std::set<pos, pos_comparer> pos_set;
typedef std::map<pos, int, pos_comparer> pos_map;


class hiyoko {
public:
  static const unsigned char suji() { return 0x40; /* 0b010_0_0_000 */ }
  static const unsigned char reverse() { return 0xfa; /* 0b111_1_1_010 */ }
  static const char display() { return 'H'; }
};
class elephant {
public:
  static const unsigned char suji() { return 0xa5; /* 0b101_0_0_101 */ }
  static const unsigned char reverse() { return 0; }
  static const char display() { return 'E'; }
};
class giraffe {
public:
  static const unsigned char suji() { return 0x5a; /* 0b010_1_1_010 */ }
  static const unsigned char reverse() { return 0; }
  static const char display() { return 'G'; }
};
class lion {
public:
  static const unsigned char suji() { return 0xff; /* 0b111_1_1_111 */ }
  static const unsigned char reverse() { return 0; }
  static const char display() { return 'L'; }
};

template <class movetype>
class koma {
  koma_v v_;
public:
  koma() : v_(0) {}
  void set(int t, int x, int y) {
    v_ = (t << 5) + (x << 2) + y;
  }
  void move(pos new_pos) {
    v_ = (v_ & 0xf0) + (new_pos.x << 2) + new_pos.y;
  }

  koma_v value() { return v_; }
  bool const reversible() { return movetype::reverse()!=0; }
  bool const ownturn(int turn) {
    if (turn==0)  return (v_ & 32)==0;
    return (v_ & 32)!=0;
  }
  pos const position() {
    pos x = { (v_ >> 2) & 3, v_ & 3 };
    return x;
  }
  void kikisuji(pos_set& result) {
    koma_v x = (v_ >> 2) & 3;
    koma_v y = v_ & 3;
    int yy;
    bool upward = true, downward = true;

    if (v_ & 32) { // kouban
      yy = -1;
      if (y==3) upward = false;
      if (y==0) downward = false;
    } else {  // senban
      yy = +1;
      if (y==0) upward = false;
      if (y==3) downward = false;
    }

    unsigned char suji  = (v_ & 16)?movetype::reverse():movetype::suji();

    if ((suji & 0x80)!=0 && x > 0 && upward) {
      pos p = { x - 1, y - yy };
      result.insert( p );
    }
    if ((suji & 0x40)!=0 && upward) {
      pos p = { x, y - yy };
      result.insert( p );
    }
    if ((suji & 0x20)!=0 && x < 2 && upward) {
      pos p = { x + 1, y - yy };
      result.insert( p );
    }
    if ((suji & 0x10)!=0 && x > 0 ) {
      pos p = { x - 1, y };
      result.insert( p );
    }
    if ((suji & 0x08)!=0 && x < 2 ) {
      pos p = { x + 1, y };
      result.insert( p );
    }
    if ((suji & 0x04)!=0 && x > 0 && downward) {
      pos p = { x - 1, y + yy };
      result.insert( p );
    }
    if ((suji & 0x02)!=0 && downward) {
      pos p = { x , y + yy };
      result.insert( p );
    }
    if ((suji & 0x01)!=0 && x < 2 && downward) {
      pos p = { x + 1, y + yy };
      result.insert( p );
    }
  }
  void put_stage_char(char *buf, std::string &m1, std::string &m2) {
    char c = (v_ & 16)?'C':movetype::display();
    if (v_ & 32) c |= 0x20;

    int x  = v_ & 0x0f;
    if (x<12) {
      buf[x] = c;
    } else {
      if (v_ & 16) {
        m2.push_back(c);
      } else {
        m1.push_back(c);
      }
    }
  }
};

union stage {
  struct {
  koma<hiyoko>   k1;
  koma<elephant> k2;
  koma<giraffe>  k3;
  koma<lion>     k4;
  koma<hiyoko>   k5;
  koma<elephant> k6;
  koma<giraffe>  k7;
  koma<lion>     k8;
  };
  koma_v data[8];
  long long value;

  void initialize() {
    k1.set(0, 1, 2);
    k2.set(0, 0, 3);
    k3.set(0, 2, 3);
    k4.set(0, 1, 3);
    k5.set(1, 1, 1);
    k6.set(1, 2, 0);
    k7.set(1, 0, 0);
    k8.set(1, 1, 0);
  }

  template <class func_class>
  void each( func_class& c ){
    c(*this, k1, 0);
    c(*this, k2, 1);
    c(*this, k3, 2);
    c(*this, k4, 3);
    c(*this, k5, 4);
    c(*this, k6, 5);
    c(*this, k7, 6);
    c(*this, k8, 7);
  }
};
struct stage_comparer {
  bool operator()(const stage& x, const stage& y) const {
    return x.value < y.value;
  }
};
typedef std::set<stage, stage_comparer> stage_set;
typedef std::vector<stage> stage_vector;




class stage_condition {
  int turn_;
  pos_set rival_suji;
  pos_map map_;
public:
  stage_condition(int turn) : turn_(turn) {}
  pos_set& kikisuji() { return rival_suji; }
  pos_map& map() { return map_; }

  template <class movetype>
  void operator()(const stage& now_stage, koma<movetype>& x, int index) {
    map_[x.position()] = index;
    if (!x.ownturn(turn_)) x.kikisuji(rival_suji);
  }
};

class walk_step {
  int turn_;
  stage_vector::iterator& it_;
  stage_condition &cond_;
public:
  walk_step(stage_vector::iterator& it, stage_condition &cond, int turn) : turn_(turn), cond_(cond), it_(it) {}

  template <class movetype>
  void operator()(const stage& now_stage, koma<movetype>& x, int index) {
    if (x.ownturn(turn_)) {
      pos_set suji;
      x.kikisuji(suji);

      pos_set::const_iterator it;
      for(it = suji.begin();it != suji.end();++it) {
        koma<movetype> new_x = x;
        new_x.move(*it);

        pos p = new_x.position();
        //std::cout << (int)p.x << ", " << (int)p.y << " : " << cond_.map().count(p) << std::endl;
        if (cond_.map().count(p) == 0) { // || (now_stage.data[cond_.map()[p]] != index) {
          stage new_stage = now_stage;
          new_stage.data[index] = new_x.value();
          *it_++ = new_stage;
        }
      }
    }
  }
};

void next_step( stage_vector::iterator& prev, stage_vector::iterator& post, int turn) {
  stage_vector::iterator prev_end = post;
  while (prev != prev_end) {
    stage x = *prev;
    stage_condition cond(turn);
    x.each(cond);

    walk_step walk(post, cond, turn);
    x.each(walk);

    ++prev;
  }
}





class put_stage_char {
  char buf[12];
  std::string m1, m2;
public:
  put_stage_char() {
    std::memset(buf, 0x20, sizeof(buf));
  }
  std::string& sente() { return m1; }
  std::string& go_te() { return m2; }
  std::string display() {
    std::string s(buf, buf+4);
    s.push_back(0x0a);
    s.append(buf+4, buf+8);
    s.push_back(0x0a);
    s.append(buf+8, buf+12);
    return s;
  }

  template <class movetype>
  void operator()(const stage& now_stage, koma<movetype>& x, int index) {
    x.put_stage_char(buf, m1, m2);
  }
};

void print_stage(stage& x) {
  put_stage_char mochigoma;
  x.each(mochigoma);
  std::cout << mochigoma.display() << std::endl;
  std::cout << "[" << mochigoma.go_te() << "] [" << mochigoma.sente() << "]" << std::endl;
  
}




int main(int argc, char *argv[]) {
  assert(sizeof(stage)==8);
  stage initial;
  initial.initialize();

  stage_vector buffer;
  buffer.resize(BUFFER_SIZE);

  print_stage(initial);

  stage_vector::iterator prev = buffer.begin();
  stage_vector::iterator post = buffer.begin();
  *post++ = initial;

  next_step(prev, post, 0);
  for(stage_vector::iterator it=prev;it!=post;++it) {
    print_stage(*it);
  }

}