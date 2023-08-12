
#include <string>
#include <gtest/gtest.h>
#include <gmock/gmock.h>


using no_errors = bool; // true - is no errors

// создаём тип(а) запрос
struct DBQuery
{
	const char* data{ "query_to_database" };
	int consistency_check_marker{ false };
};

// создаём тип(а) ответ
struct DBResponce
{
	friend bool operator== (const DBResponce& responce1, const DBResponce& responce2) {
		return responce2.data == responce1.data && responce2.consistency_check_marker == responce1.consistency_check_marker;
	}
	int data{ 111 }; // просто, чтобы вернул что-нибудь (но неожиданный)
	int consistency_check_marker{ false }; // просто, чтобы вернул что-нибудь (но неожиданный)
};

// прописываем интерфейс
class DBConnectionInterface
{
public:
	DBConnectionInterface() {};
	virtual ~DBConnectionInterface() {}; // важно фигурные скобки !!!
    virtual no_errors open() =0;
    virtual no_errors close() = 0;
    virtual DBResponce execQuery(const DBQuery& query) = 0;
private:
};

// коннекция на базе интерфейса
class DBConnection : public DBConnectionInterface
{
public:
    ~DBConnection() override {} ; // важно фигурные скобки !!!
	virtual no_errors open() override { return no_errors(false); };
    virtual no_errors close() override { return no_errors(false); };
    virtual DBResponce execQuery(const DBQuery& query) override { return queryToResponce(query); };

private:
    		DBResponce queryToResponce(const DBQuery& query) {
			DBResponce responce;
			responce.consistency_check_marker = query.consistency_check_marker + 1;
			responce.data = 0xFFFF;
			return responce;
		};
};

// макет коннекции на базе интерфейса
class DBConnectionMock : public DBConnectionInterface
{
public:
    ~DBConnectionMock() override {} ;
    MOCK_METHOD(no_errors, open, (), (override)) {}; // важно фигурные скобки !!!
    MOCK_METHOD(no_errors, close, (), (override)) {}; // важно фигурные скобки !!!
    MOCK_METHOD(DBResponce, execQuery, (const DBQuery&), (override)) {}; // важно фигурные скобки !!!
private:
};

// класс, который "в действительности" использует подключение к базе
class ClassThatUsesDB {
public:
    DBResponce makeQuery(DBConnectionInterface* dBConnection, const DBQuery& query) {
        openConnection(dBConnection);
        DBResponce responce = useConnection(dBConnection, query);
        closeConnection(dBConnection);
        return DBResponce(responce);
    }
private:
    no_errors openConnection(DBConnectionInterface* dBConnection) { dBConnection->open(); return no_errors(true); }
    no_errors closeConnection(DBConnectionInterface* dBConnection) { dBConnection->close(); return no_errors(true); }
    DBResponce useConnection(DBConnectionInterface* dBConnection, const DBQuery& query) { return dBConnection->execQuery(query); }
};

//создаем оболочку для тестов
class TestSuite : public ::testing::Test
{
protected:
    void SetUp()
    {
        usesDB = new ClassThatUsesDB();
    }

    void TearDown()
    {
        delete usesDB;
    }

protected:
    ClassThatUsesDB* usesDB;
};


// подключаем пространства имён
using ::testing::Expectation;
using ::testing::Return;
using ::testing::AtLeast;

//тест на правильность выполнения метода преобразования команды в массив байт
TEST_F(TestSuite, testQuery)
{
    //для целей теста формируем запрос и ожидаемый результат
    DBQuery query;
    DBResponce expected_responce;
    expected_responce.consistency_check_marker = query.consistency_check_marker + 1;
    expected_responce.data = 0xFFFF;

    // создаём "реальный" коннект
    DBConnection dBConnection;

    // проверяем makeQuery(),
    // остальные методы приватные, поэтому тест для них не вызвать
    // проверяем, что действительно возвращаемый результат соответствует ожиданиям
    DBResponce executed_responce = usesDB->makeQuery(&dBConnection, query);
    EXPECT_EQ(executed_responce, expected_responce);
}


TEST_F(TestSuite, testSequence)
{
    DBConnectionMock dBConnectionMock;
    DBQuery query;
    DBResponce responce;

    Expectation open = EXPECT_CALL(dBConnectionMock, open)
        .WillOnce(Return(no_errors(true))
        );

	Expectation execQuery = EXPECT_CALL(dBConnectionMock, execQuery)
		.Times(AtLeast(1))
		.After(open)
		.WillRepeatedly(Return(DBResponce(responce))
		);

	Expectation close = EXPECT_CALL(dBConnectionMock, close)
		.WillOnce(Return(no_errors(true)));

    usesDB->makeQuery(&dBConnectionMock, query);
}


int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}