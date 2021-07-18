#include "stringsplitter.hh"

void stringsplitter::reset()
{
  current = new node<std::string *>();
}

void stringsplitter::init(const char * str, const char * delimiter)
{
  //0. Do NULL checks
    if(str == NULL || delimiter == NULL) { return; }

  //2. Iterate through the strings making a new object as you create 
    //a. To be a prime node
      node<std::string *> head = new node<std::string *>( NULL );

    //b. Need all of these vars for iteration
      node<std::string *> node_token = head;
      std::string::size_type _found = 0;
    
      std::string current_string = str;
      std::sring delim = delimiter;
      std::string * split_token = NULL;
    
      while( ( _found = current_string.find(delim) ) != std::string::npos )
      {
        //i. Set the token value
          split_token = new std::string( current_string.substr( 0, _found ) );
        
        //ii. Make the next of node token with this holder
          node_token->next(split_token);

        //iii. Iterate the node_token
          node_token = node_token->next;
        
        //iv. Erase the string from current
          current_string.erase(0, _found + delim.length() );

      }
    //c. Set the next of node_token to the last string set if current_string is not empty
      if( !current_string.empty() )
      {
        //i. Set the token
          split_token = new std::string(current_string);
        
        //ii. Instantiate the next
          node_token->next(split_token);

        //iii. Iterate the node token
          node_token = node_token->next;
      }

    //d. Set the node_token->next to the head and the prev of head to node_token->next
      node_token->next(head);
      
    //d. Clear current_string and delim as they are no longer useful and set the tokens to NULL
      node_token = NULL;
      split_token = NULL;
      current_string.clear();
      delim.clear();

    //d. Set the current to the next of head
      current = head->next;

}

void stringsplitter::clear()
{
  node<std::string *> * temp = current;
  node<std::string *> * next = current->next;

}

stringsplitter & stringsplitter::operator++()
{
  if(current->next->data != NULL) { current = current->next; }
  else if(current->next->next->data != NULL) { current = current->next->next; }
  return *this;
}

stringsplitter & stringsplitter::operator++(int)
{
  if(current->next->data != NULL) { current = current->next; }
  else if(current->next->next->data != NULL) { current = current->next->next; }
  return *this;
}

stringsplitter & stringsplitter::operator--()
{ 
  if(current->prev->data != NULL) { current = current->prev; }
  else if(current->prev->prev->data != NULL) { current = current->prev->prev; }
  return *this;
}

stringsplitter & stringsplitter::operator--(int)
{ 
  if(current->prev->data != NULL) { current = current->prev; }
  else if(current->prev->prev->data != NULL) { current = current->prev->prev; }
  return *this;
}

std::string * operator[](size_t index)
{
  int len = 0;
  node<std::string *> * temp = current;
  do
  {
    if(len == index) { return temp; }
    temp = temp->next;
    len++;

  } while(temp->next->data != NULL);
  return NULL;
}

std::string * stringsplitter::val()
{
  if(current == NULL) { return NULL; }
  else { return current->data; }
}

void strinsplitter::output() {}
