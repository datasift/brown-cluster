#ifndef __STRDB_H__
#define __STRDB_H__

#include "std.h"
#include "stl-basic.h"
#include "stl-utils.h"
#include "logging.h"
/*
void destroy_strings(StrVec &vec);
void destroy_strings(StrStrMap &map);
*/
// Map between strings and integers.
// Strings must not have spaces in them.
// File format: strings, one per line.  Assume strings are distinct.

//



////////////////////////////////////////////////////////////
struct StrDB ;

typedef void int_func(int a);
void read_text(const std::string &file, int_func *func, StrDB &db, bool read_cached, bool write_cached, bool incorp_new);

/*
 * ? should the index be unsigned ?
 * ? is lookup needed ?
 *
 *
 */
struct StrDB {


  int read(istream &in, int n, bool one_way);
  int read(const std::string &file, bool one_way);

  void write(ostream &out) const;
  void write(const std::string &file) const;

  int size() const   { return i2s.size(); }
  void clear()       { i2s.clear(); s2i.clear(); }

  void destroy_s2i() { s2i.clear(); }


  std::string operator[](int i) const;
  int operator[](const std::string &s) const;
  int operator[](const std::string &s);
  int lookup(const std::string &s, bool incorp_new, int default_i);

  bool exists(const std::string &s) const { return s2i.find(s) != s2i.end(); }

  StringVec i2s;
  StringIntMap s2i;

  int insert_if_new(std::string &&s);
  int insert_if_new(const std::string &s);

  /*
   * \brief if found, return index, otherwise return -1

   */
  int lookup(const std::string &s) const;


/*
  StrDB() { }

  ~StrDB() { destroy_strings(); }
  void destroy()     { destroy_strings(); ::destroy(i2s); ::destroy(s2i); }
  void destroy_s2i() { ::destroy(s2i); }
  void clear_keep_strings() { i2s.clear(); s2i.clear(); }
  int read(const char *file, bool one_way);

  // /usr/bin/top might not show the memory reduced.
  void destroy_strings() { ::destroy_strings(i2s); }
 */



};
// ostream &operator<<(ostream &out, const StrDB &db);



#endif
