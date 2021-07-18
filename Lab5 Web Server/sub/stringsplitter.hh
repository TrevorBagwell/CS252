#ifndef STRINGSPLITTER_HH
#define STRINGSPLITTER_HH

#include <string>


#ifndef NODE_HH
#define NODE_HH

template <typename T>
struct node
{
  //Variables  
    node<T> * prev;
    T data;
    node<T> * next;
    
  //Constructor 0
    node() { reset(); }
    reset() { prev = NULL; next = NULL; }

  //Constructor 1
    node(T _data) { reset(); init(_data); }
    void init(T _data) { data = _data; }
 
  //Constructor 2
    node(node * _prev, T _data) { prev = _prev; data = _data; }

  //Destructor 0
    ~node() { clear(); }
    void clear() { remove(); prev = reset(); }

  //Function 0
    void next(T _data) { next = new node<T>(this,_data); }
  
  //Function 1
    void next(node<T> * nxt) { this->next = nxt; if(nxt != NULL) { nxt->prev = this; } }
  //Function 2
    void remove() 
    { 
      if(prev != NULL) { prev->next = next; } 
      prev = NULL;

      if(next != NULL) { next->prev = prev; } 
      next = NULL;
    }    
};

#endif

struct stringsplitter
{
  node<std::string *> * current;
  
  //Constructor 0
    stringsplitter() { reset(); }
    void reset(); 

  //Constructor 2
    stringsplitter(const char * str, const char * delimiter) { init(str, delimiter); }
    void init(const char * str, const char * delimiter);
  
  //Destructor 0
   ~stringsplitter() { clear(); }
   void clear();
  
  
  //Operator 0
    stringsplitter & operator++();
  
  //Operator 1
    stringsplitter & operator++(int);
  
  //Operator 2
    stringsplitter & operator--();
  
  //Operator 3
    stringsplitter & operator--(int);
  
  //Operator 4
    std::string * operator[](size_t index);


  //Function 0
    std::string * val();
  
  //Function 1
    void output();
};



#endif
