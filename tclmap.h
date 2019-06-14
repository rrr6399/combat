// -*- c++ -*-

/*
 * ======================================================================
 *
 * This file is part of Combat, the Tcl interface for CORBA
 * Copyright (c) Frank Pilhofer
 *
 * ======================================================================
 */
 
/*
 * Implementation of a simple STL lookalike map type using a Tcl hash table.
 * Should be much faster than the ministl implementation.
 */

#ifndef __TCLMAP_H__
#define __TCLMAP_H__

#include <tcl.h>

template<class vT> class TclStringMap;
template<class kT, class vT> class TclIntegerMap;

template<class kT, class vT>
struct TclPair {
  kT first;
  vT & second;
  TclPair(kT a, vT &b) : first(a), second(b) {}
};

template<class kT, class vT>
class TclMapIterator {
  friend class TclStringMap<vT>;
  friend class TclIntegerMap<kT, vT>;

private:
  Tcl_HashTable * table;
  Tcl_HashEntry * entry;
  Tcl_HashSearch search;

  TclMapIterator (Tcl_HashTable *_t)
    : table (_t)
  {
    entry = Tcl_FirstHashEntry (table, &search);
  }

  TclMapIterator (Tcl_HashTable *_t, Tcl_HashEntry *_e)
    : table (_t), entry (_e)
  {
  }

public:
  TclMapIterator ()
  {
    entry = NULL;
  }

  bool operator== (const TclMapIterator<kT, vT> & other)
  {
    return entry == other.entry;
  }

  bool operator!= (const TclMapIterator<kT, vT> & other)
  {
    return !operator== (other);
  }

  TclMapIterator<kT, vT> operator++ ()
  {
    entry = Tcl_NextHashEntry (&search);
    return *this;
  }

  TclMapIterator<kT, vT> operator++ (int)
  {
    TclMapIterator<kT, vT> tmp = *this;
    entry = Tcl_NextHashEntry (&search);
    return tmp;
  }

  TclPair<kT, vT> operator* ()
  {
    return TclPair<kT, vT> ((kT) Tcl_GetHashKey (table, entry),
			    *((vT *) (void *) Tcl_GetHashValue (entry)));
  }
};

/*
 * Map using strings as keys
 */

template<class vT>
class TclStringMap {
public:
  typedef TclMapIterator<const char *, vT> iterator;

private:
  Tcl_HashTable table;

public:
  TclStringMap ()
  {
    Tcl_InitHashTable (&table, TCL_STRING_KEYS);
  }

  ~TclStringMap ()
  {
    Tcl_HashSearch search;
    Tcl_HashEntry * entry = Tcl_FirstHashEntry (&table, &search);
    while (entry) {
      delete (vT *) (void *) Tcl_GetHashValue (entry);
      entry = Tcl_NextHashEntry (&search);
    }
    Tcl_DeleteHashTable (&table);
  }

  iterator begin ()
  {
    return iterator (&table);
  }

  iterator end ()
  {
    return iterator ();
  }

  bool empty ()
  {
    Tcl_HashSearch search;
    Tcl_HashEntry * entry = Tcl_FirstHashEntry (&table, &search);
    return entry == NULL;
  }

  size_t size ()
  {
    Tcl_HashSearch search;
    size_t count=0;
    Tcl_HashEntry * entry = Tcl_FirstHashEntry (&table, &search);
    while (entry) {
      count++;
      entry = Tcl_NextHashEntry (&search);
    };
    return count;
  }

  void insert (const char * key, const vT & value)
  {
    Tcl_HashEntry * entry;
    int isnew;
    entry = Tcl_CreateHashEntry (&table, (char *) key, &isnew);
    if (!isnew) {
      delete (vT *) (void *) Tcl_GetHashValue (entry);
    }
    Tcl_SetHashValue (entry, (void *) new vT (value));
  }

  void erase (iterator it)
  {
    if (it.entry) {
      delete (vT *) (void *) Tcl_GetHashValue (it.entry);
      Tcl_DeleteHashEntry (it.entry);
    }
  }

  void erase (const char * key)
  {
    Tcl_HashEntry * entry = Tcl_FindHashEntry (&table, (char *) key);
    if (entry) {
      delete (vT *) (void *) Tcl_GetHashValue (entry);
      Tcl_DeleteHashEntry (entry);
    }
  }
  
  iterator find (const char * key)
  {
    Tcl_HashEntry * entry = Tcl_FindHashEntry (&table, (char *) key);
    return iterator (&table, entry);
  }

  bool exists (const char * key)
  {
    Tcl_HashEntry * entry = Tcl_FindHashEntry (&table, (char *) key);
    return entry != NULL;
  }

  vT &operator[] (const char * key)
  {
    Tcl_HashEntry * entry;
    int isnew;
    entry = Tcl_CreateHashEntry (&table, (char *) key, &isnew);
    if (isnew) {
      Tcl_SetHashValue (entry, (void *) new vT);
    }
    return *((vT *) (void *) Tcl_GetHashValue (entry));
  }
};

/*
 * Map using integers as keys
 */

template<class kT, class vT>
class TclIntegerMap {
public:
  typedef TclMapIterator<kT, vT> iterator;

private:
  Tcl_HashTable table;

public:
  TclIntegerMap ()
  {
    Tcl_InitHashTable (&table, TCL_ONE_WORD_KEYS);
  }

  ~TclIntegerMap ()
  {
    Tcl_HashSearch search;
    Tcl_HashEntry * entry = Tcl_FirstHashEntry (&table, &search);
    while (entry) {
      delete (vT *) (void *) Tcl_GetHashValue (entry);
      entry = Tcl_NextHashEntry (&search);
    }
    Tcl_DeleteHashTable (&table);
  }

  iterator begin ()
  {
    return iterator (&table);
  }

  iterator end ()
  {
    return iterator ();
  }

  bool empty ()
  {
    Tcl_HashSearch search;
    Tcl_HashEntry * entry = Tcl_FirstHashEntry (&table, &search);
    return entry == NULL;
  }

  size_t size ()
  {
    Tcl_HashSearch search;
    size_t count=0;
    Tcl_HashEntry * entry = Tcl_FirstHashEntry (&table, &search);
    while (entry) {
      count++;
      entry = Tcl_NextHashEntry (&search);
    };
    return count;
  }

  void insert (kT key, const vT & value)
  {
    Tcl_HashEntry * entry;
    int isnew;
    entry = Tcl_CreateHashEntry (&table, (char *) key, &isnew);
    if (!isnew) {
      delete (vT *) (void *) Tcl_GetHashValue (entry);
    }
    Tcl_SetHashValue (entry, (void *) new vT (value));
  }

  void erase (iterator it)
  {
    if (it.entry) {
      delete (vT *) (void *) Tcl_GetHashValue (it.entry);
      Tcl_DeleteHashEntry (it.entry);
    }
  }

  void erase (kT key)
  {
    Tcl_HashEntry * entry = Tcl_FindHashEntry (&table, (char *) key);
    if (entry) {
      delete (vT *) (void *) Tcl_GetHashValue (entry);
      Tcl_DeleteHashEntry (entry);
    }
  }
  
  iterator find (kT key)
  {
    Tcl_HashEntry * entry = Tcl_FindHashEntry (&table, (char *) key);
    return iterator (&table, entry);
  }

  bool exists (kT key)
  {
    Tcl_HashEntry * entry = Tcl_FindHashEntry (&table, (char *) key);
    return entry != NULL;
  }

  vT &operator[] (kT key)
  {
    Tcl_HashEntry * entry;
    int isnew;
    entry = Tcl_CreateHashEntry (&table, (char *) key, &isnew);
    if (isnew) {
      Tcl_SetHashValue (entry, (void *) new vT);
    }
    return *((vT *) (void *) Tcl_GetHashValue (entry));
  }
};

#endif
