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

class Counter : public Process {
public:
	void Run(void) {
		int counter = 0;
		while (1) {
			Message *m;
			m = ReceiveMessage(__FILE__, __LINE__);

			if ( typeid(*m) == typeid(AddMessage) ) {
				++counter;
				delete m;
			}
			else if ( typeid(*m) == typeid(GetMessage) ) {
				m->m_sender->QueueMessage(
					new ValueMessage<int>(this, counter));
				delete m;
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
		Message *m = ReceiveMessage(__FILE__, __LINE__);
		printf("%d\n",dynamic_cast<ValueMessage<int>*>(m)->value);
		c->QueueMessage(new AddMessage(this));
		c->QueueMessage(new GetMessage(this));
		m = ReceiveMessage(__FILE__, __LINE__);
		printf("%d\n",dynamic_cast<ValueMessage<int>*>(m)->value);
	}
};

int main(int argc, char *argv[])
{
	Main m;
	m.Run();
	m.Wait();
}
