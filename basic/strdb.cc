#include "strdb.h"
#include "str.h"

int StrDB::read(const string &file, bool one_way) {
  track("StrDB::read()", file << ", one_way=" << one_way, true);
  std::ifstream in(file);
  assert(in);
  return read(in, INT_MAX, one_way);
}

void StrDB::write(const std::string &file) const {
  track("StrDB::write()", file, true);
  std::ofstream out(file);
  write(out);
}

int StrDB::insert_if_new(std::string &&s){

	StringIntMap::const_iterator it = s2i.find(s);
	if(it != s2i.end()) return it->second;

	int i = s2i[s] = i2s.size();
	i2s.push_back(std::move(s));
	return i;
}

int StrDB::insert_if_new(const std::string &s){

	StringIntMap::const_iterator it = s2i.find(s);
	if(it != s2i.end()) return it->second;

	int i = s2i[s] = i2s.size();
	i2s.push_back(s);
	return i;
}

std::string StrDB::operator[](int i) const {
  assert(i >= 0 && i < i2s.size());
  return i2s[i];
}

int StrDB::read(istream &in, int N, bool one_way) {
  std::string s;
  clear();
  while(size() < N && in >> s) {
    if(one_way) i2s.push_back(std::move(s));
    else insert_if_new(std::move(s));
  }
  logs(size() << " strings read");
  return size();
}

void StrDB::write(ostream &out) const {
  std::for_each(i2s.begin(), i2s.end(),[&](const std::string & element) { out << element << std::endl;});
  logs(size() << " strings written");
}
/*
int StrDB::lookup(const std::string &s,  int default_i) {

	StringIntMap::const_iterator it = s2i.find(s);
	if(it != s2i.end()) return it->second;
    return default_i;
}*/

int StrDB::lookup(const std::string &s) {

	StringIntMap::const_iterator it = s2i.find(s);
	if(it != s2i.end()) return it->second;
    return -1;
}

IntVec StrDB::lookup(const StringVec &svec) {
  IntVec ivec(svec.size());
  for_each(svec.begin(), svec.end(), [&](const std::string&s){
    										ivec.push_back( insert_if_new(s));
  	  	  	  	  	  	  	  	  	  	  });
  return ivec;
}

ostream &operator<<(ostream &out, const StrDB &db) {
  db.write(out);
  return out;
}

///////


void destroy_strings(StringVec &vec) {
  foridx(i, len(vec))
    delete [] vec[i];
}

void destroy_strings(StrStrMap &map) {
  typedef const char *const_char_ptr;
  StringVec strs;
  formap(const_char_ptr, s, const_char_ptr, t, StrStrMap, map) {
    strs.push_back(s);
    strs.push_back(t);
  }
  destroy_strings(strs);
}
/*
const char *StrDB::operator[](int i) const {
  assert(i >= 0 && i < len(i2s));
  return i2s[i];
}


int StrDB::lookup(const char *s, bool incorp_new, int default_i) {
  StringIntMap::const_iterator it = s2i.find(s);
  if(it != s2i.end()) return it->second;
  if(incorp_new) {
    char *t = copy_str(s);
    int i = s2i[t] = len(i2s);
    i2s.push_back(t);
    return i;
  }
  else
    return default_i;
}


int StrDB::operator[](const char *s) const {
  StringIntMap::const_iterator it = s2i.find(s);
  if(it != s2i.end()) return it->second;
  return -1;
}

int StrDB::operator[](const char *s) {
  return lookup(s, true, -1);
}

*/

////////////////////////////////////////////////////////////

int IntPairIntDB::lookup(const IntPair &p, bool incorp_new, int default_i) {
  IntPairIntMap::const_iterator it = p2i.find(p);
  if(it != p2i.end()) return it->second;

  if(incorp_new) {
    int i = p2i[p] = len(i2p);
    i2p.push_back(p);
    return i;
  }
  else
    return default_i;
}

int IntPairIntDB::read(istream &in, int N) {
  assert(size() == 0);
  int a, b;
  while(size() < N && in >> a >> b)
    (*this)[IntPair(a, b)];
  return size();
}

void IntPairIntDB::write(ostream &out) {
  forvec(_, const IntPair &, p, i2p)
    out << p.first << ' ' << p.second << endl;
}

////////////////////////////////////////////////////////////

int IntVecIntDB::lookup(const IntVec &v, bool incorp_new, int default_i) {
  IntVecIntMap::const_iterator it = v2i.find(v);
  if(it != v2i.end()) return it->second;

  if(incorp_new) {
    int i = v2i[v] = len(i2v);
    i2v.push_back(v);
    return i;
  }
  else
    return default_i;
}

////////////////////////////////////////////////////////////

// A text is basically a string of words.
// Normally, we just read the strings from file, put them in db,
// and call back func.
// But if the db already exists and the strings have been converted
// into integers (i.e., <file>.{strdb,int} exist), then use those.
// If incorp_new is false, then words not in db will just get passed -1.
typedef void int_func(int a);
void read_text(const char *file, int_func *func, StrDB &db, bool read_cached, bool write_cached, bool incorp_new) {
  track("read_text()", file, true);

  string strdb_file = string(file)+".strdb";
  string int_file = string(file)+".int";

  // Use the cached strdb and int files only if they exist and they are
  // newer than the text file.
  read_cached &= file_exists(strdb_file.c_str()) &&
                 file_exists(int_file.c_str()) &&
                 file_modified_time(strdb_file.c_str()) > file_modified_time(file) &&
                 file_modified_time(int_file.c_str()) > file_modified_time(file);

  if(read_cached) {
    // Read from strdb and int.
    assert(db.size() == 0); // db must be empty because we're going to clobber it all
    db.read(strdb_file, true);
    track_block("", "Reading from " << int_file, false) {
      ifstream in(int_file.c_str());
      std::string buf;
      while(true) {
        in >> buf;
        if(in.gcount() == 0) break;
        /*assert(in.gcount() % sizeof(int) == 0);
        for(int buf_i = 0; buf_i < in.gcount(); buf_i += 4) {
          int a = *((int *)(buf+buf_i));
          assert(a >= 0 && a < db.size());
          func(a);
        }*/
      }
    }
  }
  else {
    track_block("", "Reading from " << file, false) {
      // Write to strdb and int.
      ifstream in(file);
      ofstream out;
      
      if(write_cached) {
        out.open(int_file.c_str());
        if(!out) write_cached = false;
      }
      if(write_cached) logs("Writing to " << int_file);

      std::string s;
      while(in >> s) { // Read a string
        int a = db.insert_if_new(std::move(s));
        if(func) func(a);

        if(write_cached) {
           out << a << std::endl;
        }
      }
    }

    if(write_cached && create_file(strdb_file.c_str()))
      db.write(strdb_file.c_str());
  }
}
