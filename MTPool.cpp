#include "MTPool.h"
#include <vector>
#include "LifeCircleLogger.h"
using namespace std;

object_pool<CCSprite> spritePool;
static void testPoolEfficiency(){
	const int test_time = 1;
	vector<int> ops;
	int tot=0;
	for(int i=0;i<test_time;i++){
		int op=0;
		if(tot){
			if(rand()%2==0){
				op=1;
			}
		}
		if(op) tot--;
		else tot++;
		ops.push_back(op);
	}
	{
		LifeCircleLogger logger("use system new");
		CCArray *tmp = CCArray::array();
		for(int i=0;i<test_time;i++){
			if(ops[i]){
				tmp->removeLastObject();
			}else{
				CCSprite *sp = CCSprite::node();
				tmp->addObject(sp);
			}	
		}
	}{
		LifeCircleLogger logger("use MTPool");
		CCArray *tmp = CCArray::array();
		for(int i=0;i<test_time;i++){
			if(ops[i]){
				tmp->removeLastObject();
			}else{
				CCSprite *sp = MTPoolManager::sharedManager()->getObject(sp);
				tmp->addObject(sp);

			}	
		}
	}{
		LifeCircleLogger logger("use boost Pool");
		CCArray *tmp = CCArray::array();
		for(int i=0;i<test_time;i++){
			if(ops[i]){
				tmp->removeLastObject();
			}else{
				CCSprite *sp =spritePool.construct();
				tmp->addObject(sp);
			}	
		}
	}
}