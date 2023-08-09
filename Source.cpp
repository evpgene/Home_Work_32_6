
#include <string>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// some changes
// some other changes

using DB_address = std::string;
using DB_name = std::string;
using no_errors = bool; // true - is no errors

struct DB_ConnectionParameters
{
	DB_address address{ std::to_string(5011) };
	DB_name name{ " Connection Name" };
};

struct DB_Query
{
	const char* data{ "query_to_database" };
	int consistency_check_marker{ false };
};

struct DB_Responce
{
	friend bool operator== (const DB_Responce& responce1, const DB_Responce& responce2) {
		return responce2.data == responce1.data && responce2.consistency_check_marker == responce1.consistency_check_marker;
	}

	int data{ 111 };
	int consistency_check_marker{ false };
};


class DBConnectionInterface
{
public:
	DBConnectionInterface() {  };
	virtual ~DBConnectionInterface() {  };
	virtual no_errors open_con()  =0;
	virtual no_errors close_con() =0;
	virtual DB_Responce execQuery(DB_Query query) =0;


public:
	DB_ConnectionParameters param;

};

//создаем класс мок-объекта обмена
class DBConnection_Mock : public DBConnectionInterface
{
public:
	MOCK_METHOD(no_errors, open_con, (), (override));
	MOCK_METHOD(no_errors, close_con, (), (override));
	MOCK_METHOD(DB_Responce, execQuery, (DB_Query query), (override));

private:

};


class ClassThatUsesDB : public DBConnectionInterface
{
public:
	no_errors open_con() override { return true; };
	no_errors close_con() override { return true; };
	DB_Responce execQuery(DB_Query query) override { 
		open_con();
		DB_Responce responce = queryToResponce(query);
		close_con();
		return responce; };

	DB_Responce queryToResponce(DB_Query query) {
		DB_Responce responce;
		responce.consistency_check_marker = query.consistency_check_marker + 1;
		responce.data = 0xFFFF;
		return responce;
	};

};





//создаем фикстуру и тирдаун для наших тестов «на всякий случай»
class ClassThatUsesDB_TestSuite : public ::testing::Test {
protected:
	void SetUp() {
		dBConnection = new ClassThatUsesDB();
	}
	void TearDown() {
		delete dBConnection;
	}
	DBConnectionInterface* dBConnection;
};

//
//
//
//
//
//
TEST_F(ClassThatUsesDB_TestSuite, testcase2)
{
	DB_Query query;
	DB_Responce responce;
	DB_Responce expected_responce;
	expected_responce.consistency_check_marker = query.consistency_check_marker + 1;
	expected_responce.data = 0xFFFF;
	DBConnection_Mock dBConnection_Mock;


	dBConnection_Mock.execQuery(query);
	EXPECT_CALL(dBConnection_Mock, open_con).WillOnce(::testing::Return(true));
	EXPECT_CALL(dBConnection_Mock, execQuery).WillRepeatedly(::testing::Return(responce));
	EXPECT_CALL(dBConnection_Mock, close_con).WillOnce(::testing::Return(true));

	DB_Responce exec_responce = dBConnection->execQuery(query);

	//сравниваем полученный результат с референсом
	//ASSERT_EQ(exec_responce, expected_responce);
}


TEST_F(ClassThatUsesDB_TestSuite, testcase3)
{
	DB_ConnectionParameters param;
	no_errors open_result = true; //dBConnection->close_con(param);
	no_errors  check_result{ true };
	//ASSERT_EQ(check_result, open_result);
}

//





// короче тут надо создать три теста для каждой из функций 


int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}