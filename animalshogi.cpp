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
    c(k1);
    c(k2);
    c(k3);
    c(k4);
    c(k5);
    c(k6);
    c(k7);
    c(k8);
  }
};
struct stage_comparer {
  bool operator()(const stage& x, const stage& y) const {
    return x.value < y.value;
  }
};
typedef std::set<stage, stage_comparer> stage_set;


class put_stage_char {
  char buf[12];
  std::string m1, m2;
public:
  put_stage_char() {
    std::memset(buf, 0x20, sizeof(buf));
  }
  std::string& sente() { return m1; }
  std::string& go_te() { return m2; }
  std::string stage() {
    std::string s(buf, buf+4);
    s.push_back(0x0a);
    s.append(buf+4, buf+8);
    s.push_back(0x0a);
    s.append(buf+8, buf+12);
    return s;
  }

  template <class movetype>
  void operator()(koma<movetype>& x) {
    x.put_stage_char(buf, m1, m2);
  }
};

void print_stage(stage& x) {
  put_stage_char mochigoma;
  x.each(mochigoma);
  std::cout << mochigoma.stage() << std::endl;
  std::cout << "[" << mochigoma.go_te() << "] [" << mochigoma.sente() << "]" << std::endl;
  
}

class make_rival_suji {
  int turn_;
  pos_set rival_suji;
public:
  make_rival_suji(int turn) : turn_(turn) {}
  pos_set& kikisuji() { return rival_suji; }
  
  template <class movetype>
  void operator()(koma<movetype>& x) {
    if (!x.ownturn(turn_)) x.kikisuji(rival_suji);
  }
};

void next_step( std::vector<stage>::iterator& prev, std::vector<stage>::iterator& post, int turn) {
  std::vector<stage>::iterator prev_end = post;
  while (prev != prev_end) {
    stage x = *prev;
    make_rival_suji rival(turn);
    x.each(rival);

    pos_set::const_iterator it;
    for(it = rival.kikisuji().begin();it != rival.kikisuji().end();++it) {
      std::cout << (int)it->x << ", " << (int)it->y << std::endl;
    }

    *post++ = x;
    ++prev;
  }
}

int main(int argc, char *argv[]) {
  assert(sizeof(stage)==8);
  stage initial;
  initial.initialize();

  std::vector<stage> buffer;
  buffer.resize(BUFFER_SIZE);

  print_stage(initial);

  std::vector<stage>::iterator prev = buffer.begin();
  std::vector<stage>::iterator post = buffer.begin();
  *post++ = initial;

  next_step(prev, post, 0);
  for(std::vector<stage>::iterator it=prev;it!=post;++it) {
    print_stage(*it);
  }

}