#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <memory>
#include <string>

#include "src/mainWindow/baseMainWindow.h"

namespace qingliao
{
	struct DataManagerImpl;

	class DataManager
	{
	public:
		DataManager();
		~DataManager();

		// 禁止复制和移动
		DataManager(const DataManager&) = delete;
		DataManager(DataManager&&) = delete;
		DataManager& operator=(const DataManager&) = delete;
		DataManager& operator=(DataManager&&) = delete;

		static bool signUp(const std::string& email, const std::string& password, long long& user_id);
		bool signIn(long long user_id, const std::string& password);

		bool addPrivateRoom(long long user_id);
		bool romovePrivateRoom(long long user_id);
		bool addGroupRoom(long long room_id);
		bool removeGroupRoom(long long room_id);

		void addPrivateRoomMessage(long long user_id, MessageType type, const std::string& message);
		bool removePrivateRoomMessage(size_t index);
		void addGroupRoomMessage(long long group_id, long long sender_id, MessageType type, const std::string& message);
		bool removeGroupRoomMessage(size_t index);

	private:
		std::shared_ptr<DataManagerImpl> m_impl;
	};
}

#endif