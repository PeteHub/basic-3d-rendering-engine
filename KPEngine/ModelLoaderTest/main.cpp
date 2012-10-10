#include "main.h"

int main(void)
{

	KPModel *testModel = new KPModel("model\\crate.obj");
	delete testModel;

	return getchar();
}

