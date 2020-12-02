#include <v4d.h>
#include "utilities/data/EntityComponentSystem.hpp"

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
	V4D_ENTITY_DECLARE_COMPONENT(TestEntity, std::vector<int>, test3)
	V4D_ENTITY_DECLARE_COMPONENT(TestEntity, std::string, test4)
	V4D_ENTITY_DECLARE_COMPONENT(TestEntity, int, test5)
};

// TestEntity.cpp
V4D_ENTITY_DEFINE_CLASS(TestEntity)
V4D_ENTITY_DEFINE_COMPONENT(TestEntity, Test1, test1);
V4D_ENTITY_DEFINE_COMPONENT(TestEntity, Test2, test2);
V4D_ENTITY_DEFINE_COMPONENT(TestEntity, std::vector<int>, test3)
V4D_ENTITY_DEFINE_COMPONENT(TestEntity, std::string, test4)
V4D_ENTITY_DEFINE_COMPONENT(TestEntity, int, test5)

//
namespace v4d::tests {
	int EntityComponentSystem() {
		int result = 0;
		
		TestEntity::Create();
		TestEntity::Create();
		TestEntity::Create();
		
		auto entity = TestEntity::Create();
		entity->Add_test1(14, 2.5);
		if (entity->test1->a != 14) ++result;
		
		entity->Add_test2();
		entity->test2->b = 10.4;
		if (entity->test2->b != 10.4) ++result;
		
		{
			auto e = TestEntity::Create();
			e->Add_test4("hello");
			e->test4 = "world";
			e->test4.Do([&result](auto& v){
				if (v != "world") ++result;
			});
		}
		TestEntity::Create()->Add_test1(2, 7.9)->Add_test2("test")->Add_test4()->Add_test5();
		TestEntity::Create()->Add_test1(1,2.2)->Add_test1()->Add_test1()->Add_test4("texte")->Add_test5(4)->Remove_test1();
		
		{
			auto test1 = entity->test1.Lock();
			if (test1) test1->b++;
		}
		auto test1 = entity->test1.Lock();
		if (test1) test1->b++;
		test1.Unlock();
		
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
		TestEntity::Create()->Add_test3({1,2,3,4,5});
		
		// Destroy everything again
		TestEntity::ClearAll();
		
		// By now we should not have any components left so this should do nothing
		TestEntity::test1Components.ForEach([&result](int, auto& v){
			result += v.a;
		});
		
		return result;
	}
}
