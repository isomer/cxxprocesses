#include "csp.hpp"
#include <assert.h>

namespace csp {

void *Process::start_thread(void *athis)
{
	assert(athis);
	Process *p = static_cast<Process *>(athis);
	p->Run();
	/* TODO: Debug: Dump the message queue here as a "wtf?" */
	/* Esp the reject queue, we can also list __FILE__:__LINE__ of who
         * rejected it too.
         */
	return NULL;
}

#undef ReceiveMessage
Message *Process::ReceiveMessage(const char *reject_file, int reject_lineno)
{
	_Lock l(mutex);
	if (reject_lineno != last_reject_lineno || reject_file != last_reject_file)
	{
		std::copy(reject_queue.begin(), reject_queue.end(),
				std::insert_iterator< std::deque<Message *> >(
					message_queue, message_queue.begin() ) );
		reject_queue.clear();
		last_reject_lineno = reject_lineno;
		last_reject_file = reject_file;
	}
	while (message_queue.empty()) {
		pthread_cond_wait(&newitem, &mutex);
	}
	Message *r = message_queue.front();
	message_queue.pop_front();
	return r;
}

void Process::RejectMessage(Message *m)
{
	_Lock l(mutex);
	reject_queue.push_back(m);
}

void Process::Start(void)
{
	pthread_create(&thread, NULL, start_thread, this);
}

void Process::QueueMessage(Message *m)
{
	_Lock l(mutex);
	message_queue.push_back(m);
	pthread_cond_signal(&newitem);
}
}
