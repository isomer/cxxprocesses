/** \file csp.hpp
 *
 * Implemention of Communicating Sequential Processes
 *
 * Similar to erlang.
 *
 * \author Perry Lorier <perry@coders.net>
 */
#ifndef CSP_H
#define CSP_H 1
#include <deque>
#include <pthread.h>
#include <typeinfo>

namespace csp {
class Process;

/** \internal */
class _Lock {
private:
	pthread_mutex_t &m;
public:
	_Lock(pthread_mutex_t &m_) : m(m_) { pthread_mutex_lock(&m); }
	~_Lock(void) { pthread_mutex_unlock(&m); }
};

/** Base Message class
 *
 * Messages passed between processes are subclasses of this message class
 *
 * @note Data in the message should be copied, or read only, as it WILL be
 * 	 accessed from multiple threads.
 */
class Message {
public:
	Process *m_sender; /*< The process that sent this message */
	/** Construct a new Message
 	 * @param sender The process that is planning on sending this message
 	 */
	Message(Process *sender) : m_sender(sender) { };
	virtual ~Message(void) {};
};

/** A helper class that wraps a single value 
 *
 * A simple template implementation of the Message class. 
 */
template <class C>
class ValueMessage : public Message {
public:
	typedef C value_type;
	C value;
	ValueMessage(Process *sender, C value_) 
	: Message(sender), value(value_)  { };
	C operator *(void) {
		return value;
	}
	C operator ->(void) {
		return value;
	}
};

template<class C,class T>
bool has_type(T *x) { return typeid(*x) == typeid(C); }

class Process {
private:
	pthread_cond_t newitem;
	pthread_mutex_t mutex;
	pthread_t thread;
	std::deque<Message *> message_queue;
	std::deque<Message *> reject_queue;
	const char *last_reject_file;
	int last_reject_lineno;
	bool joined;
	
	static void *start_thread(void *athis);

protected:
	/** Receive a message.
 	 * \param reject_file	should be passed in __FILE__
 	 * \param reject_lineno	should be passed in __LINE__
 	 */
	Message *ReceiveMessage(const char *reject_file, int reject_lineno);
#define ReceiveMessage() ReceiveMessage(__FILE__,__LINE__)

	/** Reject handling a message at this time.
	 * \param m 	Message to reject
	 *
	 * If you don't want to handle a message right now, RejectMessage
	 * will put it back into the mailbox to be handled when a call to
	 * ReceiveMessage has a different reject_file and reject_lineno.
	 */
	void RejectMessage(Message *m);

public:
	Process(void) : 
	last_reject_file(0),
	last_reject_lineno(0),
	joined(false)
	{
		pthread_cond_init(&newitem, NULL);
		pthread_mutex_init(&mutex, NULL);
	}

	/** Starts the process */
	void Start(void);
	
	/** Overridden main method for this thread */
	virtual void Run(void) = 0;

	/** Send a message to this process */
	void QueueMessage(Message *m);

	/** Block until this process finishes */
	void Wait(void) {
		bool willjoin = false;
		{
			_Lock l(mutex);
			willjoin = !joined;
			joined=true;
		}
		if (willjoin)
			pthread_join(thread,NULL);
	}

	virtual ~Process(void) 	
	{
		bool willjoin = false;
		{
			_Lock l(mutex);
			willjoin = !joined;
			joined=true;
		}
		if (willjoin) {
			fprintf(stderr,"ERROR: Destroying a process before it's been Wait()'d for\n");
			pthread_join(thread,NULL);
		}
	}
};

}
#endif
