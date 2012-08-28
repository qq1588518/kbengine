/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ENTITY_H__
#define __ENTITY_H__
	
// common include
//#include "entitymovecontroller.hpp"
#include "cstdkbe/timer.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "pyscript/math.hpp"
#include "pyscript/scriptobject.hpp"
#include "entitydef/datatypes.hpp"	
#include "entitydef/entitydef.hpp"	
#include "entitydef/scriptdef_module.hpp"
#include "entitydef/entity_macro.hpp"	
#include "server/script_timers.hpp"	

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{
class Chunk;
class Entity;
class EntityMailbox;

namespace Mercury
{
class Channel;
}

/** 观察者信息结构 */
struct WitnessInfo
{
	WitnessInfo(const int8& lv, Entity* e, const float& r):
	detailLevel(lv),
	entity(e),
	range(r)
	{
		for(int i=0; i<3; i++)
			if(lv == i)
				detailLevelLog[i] = true;
			else
				detailLevelLog[i] = false;
	}
	
	int8 detailLevel;							// 当前所在详情级别
	Entity* entity;								// 所表达的entity
	float range;								// 当前与这个entity的距离
	bool detailLevelLog[3];						// 表示这个entity都进入过该entity的哪些详情级别， 提供属性广播优化用的
												// 当没有进入过某级别时， 会将所有这个级别的属性更新给他， 否则只更新近段时间曾经改变过的属性
	std::vector<uint32> changeDefDataLogs[3];	// entity离开了某个详情级别(没有脱离witness)后， 这期间有某个详情级别的属性改变均记录在这里
};

class Entity : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	BASE_SCRIPT_HREADER(Entity, ScriptObject)	
	ENTITY_HEADER(Entity)
public:
	typedef std::map<ENTITY_ID, WitnessInfo*>	WITNESSENTITY_DETAILLEVEL_MAP;
public:
	Entity(ENTITY_ID id, const ScriptDefModule* scriptModule);
	~Entity();
	
	/** 
		销毁这个entity 
	*/
	void onDestroy(void);
	void destroy();
	
	/** 
		判断自身是否是一个realEntity 
	*/
	INLINE bool isReal(void)const;
	
	/** 
		定义属性数据被改变了 
	*/
	void onDefDataChanged(const PropertyDescription* propertyDescription, 
			PyObject* pyData);
	
	/** 
		该entity通信通道
	*/
	INLINE void pChannel(Mercury::Channel* pchannel);
	INLINE Mercury::Channel* pChannel(void)const ;
public:
	/** 
		脚本请求销毁实体
	*/
	void destroyEntity(void);
	DECLARE_PY_MOTHOD_ARG0(pyDestroyEntity);
	
	/** 
		mailbox section
	*/
	INLINE EntityMailbox* getBaseMailbox()const;
	DECLARE_PY_GET_MOTHOD(pyGetBaseMailbox);
	INLINE void setBaseMailbox(EntityMailbox* mailbox);
	
	INLINE EntityMailbox* getClientMailbox()const;
	DECLARE_PY_GET_MOTHOD(pyGetClientMailbox);
	INLINE void setClientMailbox(EntityMailbox* mailbox);

	/** 
		脚本获取和设置entity的position 
	*/
	INLINE Position3D& getPosition();
	void setPosition(Position3D& pos);
	DECLARE_PY_GETSET_MOTHOD(pyGetPosition, pySetPosition);

	/** 
		脚本获取和设置entity的方向 
	*/
	INLINE Direction3D& getDirection();
	INLINE void setDirection(Direction3D& dir);
	DECLARE_PY_GETSET_MOTHOD(pyGetDirection, pySetDirection);
	
	/** 
		设置entity方向和位置 
	*/
	void setPositionAndDirection(Position3D& position, 
		Direction3D& direction);

	/** 网络接口
		客户端设置新位置
	*/
	void setPosition_XZ_int(Mercury::Channel* pChannel, int32 x, int32 z);

	/** 网络接口
		客户端设置新位置
	*/
	void setPosition_XYZ_int(Mercury::Channel* pChannel, int32 x, int32 y, int32 z);

	/** 网络接口
		客户端设置位置
	*/
	void setPosition_XZ_float(Mercury::Channel* pChannel, float x, float z);

	/** 网络接口
		客户端设置位置
	*/
	void setPosition_XYZ_float(Mercury::Channel* pChannel, float x, float y, float z);

	/** 
		脚本请求为当前所在space设置一个几何映射 
	*/
	DECLARE_PY_MOTHOD_ARG2(pyAddSpaceGeometryMapping, SPACE_ID, const_charptr);

	/** 
		当前entity设置自身的Aoi半径范围 
	*/
	int32 setAoiRadius(float radius, float hyst);
	INLINE float getAoiRadius(void)const;
	INLINE float getAoiHystArea(void)const;
	DECLARE_PY_MOTHOD_ARG2(pySetAoiRadius, float, float);
	
	/** 
		当前entity是否为real 
	*/
	DECLARE_PY_MOTHOD_ARG0(pyIsReal);

	/** 
		脚本获得当前entity是否为将要销毁的entity 
	*/
	INLINE bool isDestroyed();
	DECLARE_PY_GET_MOTHOD(pyGetIsDestroyed);
	
	/** 
		向baseapp发送备份数据
	*/
	void backupCellData();

	/** 
		将要保存到数据库之前的通知 
	*/
	void onWriteToDB();

	/** 
		entity移动导航 
	*/
	bool navigateStep(const Position3D& destination, float velocity, 
					float maxMoveDistance, float maxDistance, 
					bool faceMovement, float girth, PyObject* userData);

	DECLARE_PY_MOTHOD_ARG7(pyNavigateStep, PyObject_ptr, float, float, float, int8, float, PyObject_ptr);
	
	/** 
		停止任何方式的移动行为 
	*/
	bool stopMove();
	DECLARE_PY_MOTHOD_ARG0(pyStopMove);

	/** 
		entity移动到某个点 
	*/
	bool moveToPoint(const Position3D& destination, float velocity, 
			PyObject* userData, bool faceMovement, bool moveVertically);
	
	DECLARE_PY_MOTHOD_ARG5(pyMoveToPoint, PyObject_ptr, float, PyObject_ptr, int32, int32);

	
	/** 
		脚本获取和设置entity的最高xz移动速度 
	*/
	float getTopSpeed()const{ return topSpeed_; }
	INLINE void setTopSpeed(float speed);
	DECLARE_PY_GETSET_MOTHOD(pyGetTopSpeed, pySetTopSpeed);
	
	/** 
		脚本获取和设置entity的最高y移动速度 
	*/
	INLINE float getTopSpeedY()const;
	INLINE void setTopSpeedY(float speed);
	DECLARE_PY_GETSET_MOTHOD(pyGetTopSpeedY, pySetTopSpeedY);
	
	/** 
		脚本请求获得一定范围类的某种类型的entities 
	*/
	DECLARE_PY_MOTHOD_ARG3(pyEntitiesInRange, float, PyObject_ptr, PyObject_ptr);
public:
	/** 网络接口
		远程呼叫本entity的方法 
	*/
	void onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s);

	/** 
		设置这个entity当前所在chunk的ID 
	*/
	//void setCurrentChunk(Chunk* chunk){ Chunk* oldchunk = currChunk_; currChunk_ = chunk; onCurrentChunkChanged(oldchunk); }

	/** 
		当前chunk被改变 
	*/
	//void onCurrentChunkChanged(Chunk* oldChunk);

	/** 
		获得这个entity当前所在chunk的ID 
	*/
	//Chunk* getAtChunk(void)const{ return currChunk_; }

	/** 
		是否被任何proxy监视到, 如果这个entity没有客户端， 则这个值有效 
	*/
	INLINE bool isWitnessed(void)const;
	DECLARE_PY_GET_MOTHOD(pyIsWitnessed);

	/** 
		entity是否是一个观察者 
	*/
	INLINE bool hasWitness(void)const;
	DECLARE_PY_GET_MOTHOD(pyHasWitness);

	/** 
		自身被一个观察者观察到了 
	*/
	void onWitnessed(Entity* entity, float range);

	/** 
		移除一个观察自身的观察者 
	*/
	void onRemoveWitness(Entity* entity);

	/** 
		这个entity获得了观察者身份 
	*/
	void onGetWitness(void);

	/** 
		entity丢失了观察者身份 
	*/
	void onLoseWitness(void);

	/** 
		获得所观察到的entities列表 
	*/
	INLINE std::map<ENTITY_ID, Entity*>& getViewEntities(void);

	/** 
		获得所有观察到我的观察者 
	*/
	WITNESSENTITY_DETAILLEVEL_MAP& getWitnessEntities(void)
	{
		return witnessEntityDetailLevelMap_; 
	}

	/** 
		更新witness的状态 
	*/
	void onUpdateWitness(Entity* entity, float range);

	/** 
		一个新进入视野范围的entity 
	*/
	void onViewEntity(Entity* entity);

	/** 
		一个entity离开了视野范围 
	*/
	void onLoseViewEntity(Entity* entity);

	/** 
		一个entity第一次被设置(第一次进入到这个区域中)detailLevel级别 
	*/
	void onEntityInitDetailLevel(Entity* entity, int8 detailLevel);

	/** 
		一个entity因为移动改变了它在本entity的detailLevel的级别 
	*/
	void onEntityDetailLevelChanged(const WitnessInfo* witnessInfo, 
		int8 oldDetailLevel, int8 newDetailLevel);
	

	/** 
		添加一个陷阱 
	*/
	uint16 addProximity(float range);
	DECLARE_PY_MOTHOD_ARG1(pyAddProximity, float);

	/** 
		删除一个陷阱 
	*/
	void delProximity(uint16 id);
	DECLARE_PY_MOTHOD_ARG1(pyDelProximity, uint16);

	/** 
		一个entity进入了这个entity的某个陷阱 
	*/
	void onEnterTrap(Entity* entity, float range, 
							int controllerID);

	/** 
		一个entity离开了这个entity的某个陷阱 
	*/
	void onLeaveTrap(Entity* entity, float range, 
							int controllerID);

	/** 
		当entity跳到一个新的space上去后，离开陷阱陷阱事件将触发这个接口 
	*/
	void onLeaveTrapID(ENTITY_ID entityID, 
							float range, int controllerID);

	/** 
		获得陷阱管理器 
	*/
//	ProximityMgr& getTrapMgr(void){ return trapMgr_; }

	/** entity的一次移动完成 */
	void onMove(PyObject* userData);
protected:
	EntityMailbox*							clientMailbox_;						// 这个entity的客户端mailbox
	EntityMailbox*							baseMailbox_;						// 这个entity的baseapp mailbox

	Position3D								position_;							// entity的当前位置
	Direction3D								direction_;							// entity的当前方向

//	Chunk*									currChunk_;							// 这个当前entity所在的chunk

	bool									isReal_;							// 自己是否是一个realEntity

	bool									isDestroyed_;						// 自身是否将要销毁

	float									aoiRadius_;							// 当前entity的aoi半径
	float									aoiHysteresisArea_;					// 当前entityAoi的一个滞后范围

	bool									isWitnessed_;						// 是否被任何观察者监视到
	bool									hasWitness_;						// 这个entity是否是一个观察者
	std::map<ENTITY_ID, Entity*>			viewEntities_;						// 自身视野范围内的entityID， entity必须是一个观察者才有这个属性存在
	WITNESSENTITY_DETAILLEVEL_MAP			witnessEntityDetailLevelMap_;		// 这个变量记录了一个观察者在当前entity的详情级别
	std::map<ENTITY_ID, Entity*>			witnessEntities_[4];				// 观察者entity的ID列表， 被保存在一个详情级别队列里，详情级别总共4级， 分：近中远, 超远

//	ProximityMgr							trapMgr_;							// entity陷阱管理器

	float									topSpeed_;							// entity x,z轴最高移动速度
	float									topSpeedY_;							// entity y轴最高移动速度
	};																										
																											

}

#ifdef CODE_INLINE
#include "entity.ipp"
#endif
#endif
