#include "cocos2d.h"
#include "boost/pool/object_pool.hpp"
using namespace boost;
USING_NS_CC;

template<class T>
class MTPool {
protected:
	T* memory;
	CCArray* pool;
	CCArray* freePool;
	object_pool<T> objectPool;

	CCObject* getFreeObject(){
		CCObject*pRet=NULL;
		if(freePool->count()==0){
			collectObjects();
		}
		if(freePool->count()!=0){
			pRet = freePool->lastObject();
			pRet->retain();
			freePool ->removeLastObject();
			pRet->autorelease();
		}
		return pRet;
	}
	T* makeNewObject(){
		return objectPool.construct();
	}
	MTPool(){
		const int originSize = 100;
		pool = CCArray::arrayWithCapacity(originSize);	pool->retain();
		freePool = CCArray::arrayWithCapacity(originSize);	freePool->retain();
		memory = new T[originSize];
		for(int i=0;i<originSize;i++){
			T *pObj = memory+i;
			freePool->addObject(pObj);
		}
	}
	void pushObject(T* pObj){
		pool->addObject(pObj);
	}
public:
	~MTPool(){
		pool->removeAllObjects();			pool->release();
		freePool->removeAllObjects();		freePool->release();
	}
	static MTPool* sharedPool(){
		static MTPool __sharedPool;
		return &__sharedPool;
	}
	void collectObjects(){
		CCArray *tmp = CCArray::arrayWithCapacity(pool->count());
		int count =pool->count();
		for(int i=0;i<count;i++){
			CCObject *pObj = pool->objectAtIndex(i);
			if(pObj->retainCount()==1){
				freePool->addObject(pObj);
				tmp->addObject(pObj);
			}
		}
		pool->removeObjectsInArray(tmp);
	}
	void freeObjects(){
		collectObjects();
		freePool->removeAllObjects();
	}
	T* getObject(){
		T* pObj = dynamic_cast<T*>( getFreeObject() );
		if(pObj == NULL){
			pObj = makeNewObject();
		}
		this->pushObject(pObj);
		return pObj;
	}
};

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









