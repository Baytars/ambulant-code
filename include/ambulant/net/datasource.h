/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */
 
#ifndef AMBULANT_NET_DATASOURCE_H
#define AMBULANT_NET_DATASOURCE_H

#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/unix/unix_mtsync.h"
#include "ambulant/lib/unix/unix_event_processor.h"

// temporary debug messages
#include <iostream>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <cstring>


namespace ambulant {

namespace net {

namespace datasource {
	
typedef char bytes; 

class databuffer  
{
 private: 
     // duh, pointer to the buffer.
	char *m_buffer; 			
		     
	// the size of the bufer.
	int m_size;  			
	
	// how many bytes are in the buffer 														 
	int m_used;							
										
	// shift down all data above pos
	void shift_down(int pos);										
 public:
	// constructors
	databuffer();				
	databuffer(int size);	

	databuffer(databuffer& buf);    	  
	
	// destructor
	~databuffer();
	
	void resize(int newsize);
	void show(bool verbose);									// show information about the buffer, if verbose is true the buffer is dumped to cout;
	void get_data(char *data, int size); 				//retrieve data from buffer,  still thinking about arguments.
																				//there should be something to pass on error messages.
	void put_data(char *data , int size);			 	// this one puts data alway at the end.						 
	
	int used();
};


class active_datasource;

class passive_datasource : public ambulant::lib::ref_counted 
{
public:
	
	passive_datasource(char *url);
	//	passive_datasource(passive_datasource& ds);
	
	~passive_datasource();
	
	active_datasource *activate();
	
	////////////////////////
	// lib::ref_counted interface implementation
   long add_ref() {return ++m_refcount;}

	long release() {
		std::cout << "active_skeleton.release, count=" << m_refcount << std::endl;
		if(--m_refcount == 0){
			delete this;
			return 0;
		}
		return m_refcount;
	}

	long get_ref_count() const {return m_refcount;}
	
private:
	char *m_url;
	ambulant::lib::basic_atomic_count<ambulant::lib::unix::critical_section> m_refcount;
};

 
class active_datasource : public ambulant::lib::ref_counted {  	
public:
	// constructors 
	active_datasource();
	active_datasource(passive_datasource *const source,std::FILE *file);
	 // destructor
	~active_datasource();


	void start(ambulant::lib::unix::event_processor *evp,ambulant::lib::event *readdone);


	//  Get data from buffer and put 'size' bytes in buffer.
	void read(char *data, int size);

	// Return the amount of data currently in buffer.
	int size();
	
	////////////////////////
	// lib::ref_counted interface implementation
   long add_ref() {return ++m_refcount;}

	long release() {
		std::cout << "active_skeleton.release, count=" << m_refcount << std::endl;
		if(--m_refcount == 0){
			delete this;
			return 0;
		}
		return m_refcount;
	}

	long get_ref_count() const {return m_refcount;}

private:
    databuffer *buffer;
    passive_datasource *m_source;
    ambulant::lib::basic_atomic_count<ambulant::lib::unix::critical_section> m_refcount;
	int m_filesize;
	FILE *m_stream;
	void filesize();
    void read_file();

};

} //end namespace datasource

} // end namespace net

} //end namespace ambulant


#endif  //AMBULANT_NET_DATASOURCE_H
