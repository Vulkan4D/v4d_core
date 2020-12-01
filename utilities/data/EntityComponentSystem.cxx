#include <v4d.h>

struct Test1 {
	int a;
	double b;
	Test1() {}
	Test1(int a, double b) : a(a), b(b) {}
};

struct Test2 {
	std::string str;
	int a;
	double b;
	Test2() {}
	Test2(std::string s) : str(s) {}
};

// TestEntity.h
class TestEntity {
	V4D_ENTITY_DECLARE_CLASS(TestEntity)
	V4D_ENTITY_DECLARE_COMPONENT(TestEntity, Test1, test1)
	V4D_ENTITY_DECLARE_COMPONENT(TestEntity, Test2, test2)
};

// TestEntity.cpp
V4D_ENTITY_DEFINE_CLASS(TestEntity)
V4D_ENTITY_DEFINE_COMPONENT(TestEntity, Test1, test1);
V4D_ENTITY_DEFINE_COMPONENT(TestEntity, Test2, test2);

//
namespace v4d::tests {
	int EntityComponentSystem() {
		int result = 0;
		
		TestEntity::Create();
		TestEntity::Create();
		TestEntity::Create();
		
		auto entity = TestEntity::Create();
		entity->Add_test1(14, 4.5);
		if (entity->test1->a != 14) ++result;
		
		entity->Add_test2();
		entity->test2->b = 10.4;
		if (entity->test2->b != 10.4) ++result;
		
		TestEntity::Create();
		TestEntity::Create()->Add_test1(2, 7.9)->Add_test2("test");
		TestEntity::Create()->Add_test1(1,2.2)->Add_test1()->Add_test1()->Remove_test1();
		
		result += 16;
		
		TestEntity::ForEach([&result](auto& entity){
			entity->test1.Do([&result](auto& v){
				result -= v.a;
			});
		});
		
		result += 11;
		
		TestEntity::test1Components.ForEach([&result](int entityInstanceIndex, auto& v){
			if (entityInstanceIndex != 3 && entityInstanceIndex != 5) ++result;
			result -= (int)v.b;
			if (entityInstanceIndex == 5) {
				auto& test2 = TestEntity::Get(entityInstanceIndex)->test2;
				if (!test2.Do([&result](auto& t){if (t.str != "test") ++result;})) ++result;
				if (test2->str != "test") ++result;
			}
		});
		
		// Destroy everything
		entity = nullptr;
		TestEntity::ForEach([&result](auto& entity){
			entity = nullptr;
		});
		
		// By now we should not have any components left so this should do nothing
		TestEntity::test1Components.ForEach([&result](int, auto& v){
			result += v.a;
		});
		
		// Add some more again...
		TestEntity::Create()->Add_test1(1, 4.0);
		TestEntity::Create()->Add_test1(2, 5.0);
		
		// Destroy everything again
		TestEntity::ClearAll();
		
		// By now we should not have any components left so this should do nothing
		TestEntity::test1Components.ForEach([&result](int, auto& v){
			result += v.a;
		});
		
		return result;
	}
}
