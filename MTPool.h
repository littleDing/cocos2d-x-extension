#include "cocos2d.h"
#include "boost/pool/object_pool.hpp"
using namespace boost;
USING_NS_CC;

class ObjectPoolProtocol : public CCObject{
public:
	virtual void freeObjects(int maxScan=100)=0;
};

template<class T>
class MTPoolFromBoost : public ObjectPoolProtocol{
	object_pool<T> pool;
	CCArray *objects;
	MTPoolFromBoost():pool(256){
		objects=CCArray::array();
		objects->retain();
	}
public:
	~MTPoolFromBoost(){
		objects->removeAllObjects();
		objects->release();
	}
	static MTPoolFromBoost<T>* sharedPool(){
		static MTPoolFromBoost<T> __sharedPool;
		return &__sharedPool;
	}
	T* getObject(){
		T* pObj = pool.construct();
		objects->addObject(pObj);
		//pObj->release();
		return pObj;
	}
	void freeObjects(int maxScan=100){
		//return ;
		static int lastIndex =0;
		int count = objects->count();
		if(lastIndex>=count) lastIndex = 0;
		if(maxScan>count) maxScan = count;
		CCArray *toRemove = CCArray::array();
		for(int i=0;i<maxScan;i++){
			CCObject *obj = objects->objectAtIndex((i+lastIndex)%count);
			if(obj->retainCount()==2){
				toRemove->addObject(obj);
			}
		}
		objects->removeObjectsInArray(toRemove);
		for(int i=0;i<toRemove->count();i++){
			T *obj = dynamic_cast<T*>(toRemove->lastObject());
			//obj->retain();
			toRemove->removeLastObject();
			pool.destroy(obj);
		}
		CCLOG("%s ends. Obj now = %d",__FUNCTION__,objects->count());
	}
};

template <class T>
inline T* getObject(T*& pObj){
	return pObj=MTPoolFromBoost<T>::sharedPool()->getObject();
}

class MTPoolManager : public CCObject{
	CCArray *pools;
	class PoolCounter{
	public:
		PoolCounter(ObjectPoolProtocol* pool,CCArray* pools){
			pools->addObject(pool);
		}
	};
	MTPoolManager(){
		pools = CCArray::array();
		pools->retain();
	}
	~MTPoolManager(){
		pools->release();
	}
public:
	static MTPoolManager* sharedManager(){
		static MTPoolManager __sharedManager;
		return &__sharedManager;
	}
	void freeObjects(ccTime dt){
		for(int i=0;i<pools->count();i++){
			ObjectPoolProtocol* pool = dynamic_cast<ObjectPoolProtocol*>(pools->objectAtIndex(i));
			pool->freeObjects();
		}
	}
	template<class T>
	T* getObject(T*& pObj){
		static PoolCounter ___poolCounter(MTPoolFromBoost<T>::sharedPool(),pools);
		return pObj = MTPoolFromBoost<T>::sharedPool()->getObject();
	}
};

