#include <stdio.h>
#include "csp.hpp"

using namespace csp;

class AddMessage : public Message {
public:
	AddMessage(Process *p) : Message(p) { }
};

class GetMessage : public Message {
public:
	GetMessage(Process *p) : Message(p) { }
};

class QuitMessage : public Message {
public:
	QuitMessage(Process *p) : Message(p) { }
};

class Counter : public Process {
public:
	void Run(void) {
		int counter = 0;
		while (1) {
			Message *m;
			m = ReceiveMessage();

			if ( has_type<AddMessage>(m) ) {
				++counter;
				delete m;
			}
			else if ( has_type<GetMessage>(m) ) {
				m->m_sender->QueueMessage(
					new ValueMessage<int>(this, counter));
				delete m;
			}
			else if ( has_type<QuitMessage>(m) ) {
				delete m;
				break;
			}
			else RejectMessage(m);
		}
	}
};

class Main : public Process {
public:
	void Run(void) {
		Counter *c = new Counter();
		c->Start();

		c->QueueMessage(new AddMessage(this));
		c->QueueMessage(new GetMessage(this));
		Message *m = ReceiveMessage();
		printf("%d\n",dynamic_cast<ValueMessage<int>*>(m)->value);
		delete m;
		c->QueueMessage(new AddMessage(this));
		c->QueueMessage(new GetMessage(this));
		m = ReceiveMessage();
		printf("%d\n",dynamic_cast<ValueMessage<int>*>(m)->value);
		delete m;
		c->QueueMessage(new QuitMessage(this));
		c->Wait();
		delete c;
	}
};

int main(int argc, char *argv[])
{
	Main *m = new Main();
	m->Start();
	m->Wait();
	delete m;
}
