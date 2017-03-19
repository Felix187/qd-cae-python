
#include <sstream>
#include <string.h>
#include <iostream>
#include <bitset>
#include <fstream>
#include "../utility/FileUtility.hpp"
#include "D3plotBuffer.hpp"

/*
 * Constructor
 */
D3plotBuffer::D3plotBuffer(string _d3plot_path, int _wordSize){

  // Init vars
  iStateFile = 0;

  // Check File
  if(!FileUtility::check_ExistanceAndAccess(_d3plot_path)){
    throw(string("File \"" + _d3plot_path + "\" does not exist or is locked."));
  }

  this->d3plots = FileUtility::findDynaResultFiles(_d3plot_path);
  #ifdef QD_DEBUG
  cout << "Found result files:" << endl;
  for(size_t ii=0; ii<this->d3plots.size(); ++ii){
     cout << this->d3plots[ii] << endl;
  }
  cout << "End of file list." << endl;
  #endif
  //this->d3plots = FileUtility::globVector(_d3plot_path+"*");

  if(this->d3plots.size() < 1)
    throw(string("No D3plot result file could be found with the given path:"+_d3plot_path));

  this->wordSize = _wordSize;
}


/*
 * Destructor
 */
D3plotBuffer::~D3plotBuffer(){

  while( state_buffers.size() != 0 ){
    state_buffers.back().get();
    state_buffers.pop_back();
  }
  
}


/*
 * Get a char* byte buffer from the given file.
 *
 */
vector<char> D3plotBuffer::get_bufferFromFile(string filepath){

  vector<char> state_buffer;

  // Read data into buffer
  ifstream fStream;
  fStream.open(filepath.c_str(), ios::binary | ios::in);
  fStream.seekg(0,ios::end);
  long _bufferSize = fStream.tellg();
  fStream.seekg(0, ios::beg);
  //cout << "Filesize: " << *bufferSize << endl; // DEBUG
  state_buffer.reserve(_bufferSize);
  fStream.read(&state_buffer[0], _bufferSize);
  fStream.close();

  return state_buffer;
}


/*
 * get the geometry buffer
 *
 */
void D3plotBuffer::read_geometryBuffer(){
  
  this->current_buffer = D3plotBuffer::get_bufferFromFile(d3plots[0]);
};


/*
 * free the geometry buffer
 *
 */
void D3plotBuffer::free_geometryBuffer(){};


/*
 * Get the part buffer
 *
 */
void D3plotBuffer::read_partBuffer(){};


/*
 * free the part buffer
 *
 */
void D3plotBuffer::free_partBuffer(){};


/*
 * init the reading of the states
 *
 */
void D3plotBuffer::init_nextState(){

  if(this->current_buffer.size() == 0){
    this->current_buffer = D3plotBuffer::get_bufferFromFile(d3plots[0]);
  }

  // empty remaining data (prevents memory leak)
  #ifdef QD_DEBUG
  cout << "Emptying previous IO-Buffers" << endl;
  #endif
  while( state_buffers.size() != 0 ){
    state_buffers.back().get();
    state_buffers.pop_back();
  }

  // preload buffers
  for(size_t iFile=d3plots.size()-1; iFile>0; --iFile){
    state_buffers.push_back( std::async(D3plotBuffer::get_bufferFromFile,
                                         d3plots[iFile]) );
  }

}


/*
 * Get the next state buffer
 *
 */
void D3plotBuffer::read_nextState(){

  // Do not load next buffer in case of first file
  // It will be read if the end marker is hit anyways.
  // Dont ask me why LS-DYNA is so complex ... 
  if(iStateFile == 0){
    iStateFile++;
    return;
  }

  if(iStateFile >= d3plots.size()){
    throw(string("There are no more state-files to be read."));
  }

  #ifdef QD_DEBUG
  cout << "Loading state-file:" << d3plots[iStateFile] << endl;
  #endif

  this->current_buffer = state_buffers.back().get();
  state_buffers.pop_back();
  iStateFile++;
  return;
  
}


/*
 * rewind the state reading.
 *
 */
void D3plotBuffer::rewind_nextState(){

  iStateFile = 0;
  this->current_buffer = D3plotBuffer::get_bufferFromFile(d3plots[0]);

}


/*
 * check if there is a next state
 *
 */
bool D3plotBuffer::has_nextState(){
  /*
  if(iStateFile < d3plots.size())
    return true;
  */
  if(state_buffers.size() > 0)
    return true;
  return false;
}


/*
 * end the reading of states
 *
 */
void D3plotBuffer::end_nextState(){

  this->current_buffer.clear();
}

/*
 * Close the file ... just releases buffer.
 */
void D3plotBuffer::finish_reading(){

  this->current_buffer.clear();
}

/*
 * read an int from the current buffer
 */
int D3plotBuffer::read_int(int iWord){
  //if(this->bufferSize <= iWord*this->wordSize){
  //  throw("read_int tries to read beyond the buffer size.");
  //}

  // BIG ENDIAN ?
  // SMALL ENDIAN ?
  int start=iWord*this->wordSize;
  return (((current_buffer[start + 3] & 0xff) << 24)
          | ((current_buffer[ start+ 2] & 0xff) << 16)
          | ((current_buffer[start + 1] & 0xff) << 8)
          | ((current_buffer[start + 0] & 0xff)));
}


/*
 * read a float from the current buffer
 */
float D3plotBuffer::read_float(int iWord){
  //if(this->bufferSize <= iWord*this->wordSize){
  //  throw("read_float tries to read beyond the buffer size.");
  //}
  float ret;
  memcpy(&ret, &current_buffer[iWord*this->wordSize], sizeof(ret));
  return ret;
  //return (float) this->buffer[iWord*this->wordSize];
}


/*
 * read a string from the current buffer
 */
string D3plotBuffer::read_str(int iWord,int wordLength){
  //if(this->bufferSize <= (iWord+wordLength)*this->wordSize){
  //  throw("read_str tries to read beyond the buffer size.");
  stringstream res;
  for(int ii=iWord*this->wordSize;ii<(iWord+wordLength)*this->wordSize;ii++){
    res << char(bitset<8>(this->current_buffer[ii]).to_ulong());
  }

  return res.str();
}
