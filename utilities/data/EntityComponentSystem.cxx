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

struct SpecialClass {
	static int count;
	bool allocated = false;
	void Allocate() {
		allocated = true;
		++count;
	}
	void Free() {
		if (allocated) {
			allocated = false;
			--count;
		}
	}
};
int SpecialClass::count = 0;

// TestEntity.h
class TestEntity {
	V4D_ENTITY_DECLARE_CLASS(TestEntity)
	V4D_ENTITY_DECLARE_COMPONENT(TestEntity, Test1, test1)
	V4D_ENTITY_DECLARE_COMPONENT(TestEntity, Test2, test2)
	V4D_ENTITY_DECLARE_COMPONENT(TestEntity, std::vector<int>, test3)
	V4D_ENTITY_DECLARE_COMPONENT(TestEntity, std::string, test4)
	V4D_ENTITY_DECLARE_COMPONENT(TestEntity, int, test5)
	V4D_ENTITY_DECLARE_COMPONENT(TestEntity, SpecialClass, test6)
	V4D_ENTITY_DECLARE_COMPONENT_MAP(TestEntity, std::string_view, Test2, test2Map)
	
	void Allocate() {
		test6.Do([](auto& test){
			test.Allocate();
		});
	}
	
	void Free() {
		test6.Do([](auto& test){
			test.Free();
		});
	}
	
	~TestEntity() {
		Free();
	}
};

// TestEntity.cpp
V4D_ENTITY_DEFINE_CLASS(TestEntity)
V4D_ENTITY_DEFINE_COMPONENT(TestEntity, Test1, test1);
V4D_ENTITY_DEFINE_COMPONENT(TestEntity, Test2, test2);
V4D_ENTITY_DEFINE_COMPONENT(TestEntity, std::vector<int>, test3)
V4D_ENTITY_DEFINE_COMPONENT(TestEntity, std::string, test4)
V4D_ENTITY_DEFINE_COMPONENT(TestEntity, int, test5)
V4D_ENTITY_DEFINE_COMPONENT(TestEntity, SpecialClass, test6)
V4D_ENTITY_DEFINE_COMPONENT_MAP(TestEntity, std::string_view, Test2, test2Map)

//
namespace v4d::tests {
	int EntityComponentSystem() {
		int result = 0;
		
		TestEntity::Create();
		TestEntity::Create();
		TestEntity::Create();
		
		auto entity = TestEntity::Create();
		entity->Add_test1(14, 2.5);
		{auto test1 = entity->test1.Lock();
			if (test1->a != 14) ++result;
		}
		
		entity->Add_test2();
		{auto test2 = entity->test2.Lock();
			test2->b = 10.4;
			if (test2->b != 10.4) ++result;
		}
		
		{
			auto e = TestEntity::Create();
			e->Add_test4("hello");
			e->test4 = "world";
			e->test4.Do([&result](auto& v){
				if (v != "world") ++result;
			});
		}
		{
			auto e = TestEntity::Create();
			e->Add_test1(2, 7.9);
			e->Add_test2("test");
			e->Add_test4();
			e->Add_test5();
		}
		{
			auto e = TestEntity::Create();
			e->Add_test1(1,2.2);
			e->Add_test1();
			e->Add_test1();
			e->Add_test4("texte");
			e->Add_test5(4);
			e->Remove_test1();
		}
		
		{
			auto test1 = entity->test1.Lock();
			if (test1) test1->b++;
		}
		auto test1 = entity->test1.Lock();
		if (test1) test1->b++;
		test1.Unlock();
		
		result += 16;
		
		TestEntity::ForEach([&result](auto entity){
			entity->test1.Do([&result](auto& v){
				result -= v.a;
			});
		});
		
		result += 11;
		
		TestEntity::test1Components.ForEach([&result](auto entityInstanceIndex, auto& v){
			if (entityInstanceIndex != 3 && entityInstanceIndex != 5) ++result;
			result -= (int)v.b;
			if (entityInstanceIndex == 5) {
				auto& test2 = TestEntity::Get(entityInstanceIndex)->test2;
				if (!test2.Do([&result](auto& t){if (t.str != "test") ++result;})) ++result;
				auto t2 = test2.Lock();
				if (t2->str != "test") ++result;
			}
		});
		
		// Destroy everything
		entity = nullptr;
		TestEntity::ForEach([&result](auto entity){
			entity->Destroy();
		});
		
		// By now we should not have any components left so this should do nothing
		TestEntity::test1Components.ForEach([&result](auto, auto& v){
			result += v.a;
		});
		
		// Add some more again...
		TestEntity::Create()->Add_test1(1, 4.0);
		TestEntity::Create()->Add_test1(2, 5.0);
		TestEntity::Create()->Add_test3({1,2,3,4,5});
		
		// Destroy everything again
		TestEntity::ClearAll();
		
		// By now we should not have any components left so this should do nothing
		TestEntity::test1Components.ForEach([&result](auto, auto& v){
			result += v.a;
		});
		
		{// Weird stuff with destructors and multithreading
			auto e1 = TestEntity::Create();
				e1->Add_test6()->Allocate();
			auto e2 = TestEntity::Create();
				e2->Add_test6()->Allocate();
			auto e3 = TestEntity::Create();
				e3->Add_test6()->Allocate();
			auto e4 = TestEntity::Create();
				e4->Add_test6()->Allocate();
			auto e5 = TestEntity::Create();
				e5->Add_test6()->Allocate();
			auto e6 = TestEntity::Create();
				e6->Add_test6()->Allocate();
				
			result += SpecialClass::count;
			if (result != 6) return -1;
			if (TestEntity::Count() != 6) return -2;
		
			auto f1 = std::async([=](){
				e6->Destroy();
			});
			e5->Destroy();
			auto f2 = std::async([&](){
				e3->Destroy();
			});
			e4->Destroy();
			auto f3 = std::async([=](){
				e1->Destroy();
				e2->Destroy();
			});
			
			f1.get();
			f2.get();
			f3.get();
			
			result -= SpecialClass::count;
			if (result != 0) return -3;
			
			e2.reset();
			e1.reset();
			e3.reset();
			e4.reset();
			e5.reset();
			e6.reset();
			
			result += SpecialClass::count;
			if (result != 0) return -4;
		}
		
		result += TestEntity::CountActive();
		TestEntity::Trim();
		result += TestEntity::Count();
		
		// OneToMany
		{
			result = -2;
			TestEntity::Create()->Add_test2Map()["test1"] = Test2("hello");
			TestEntity::ForEach([](auto entity){
				entity->test2Map["test1"]->a = 2;
				entity->test2Map["test2"]->a = 5;
				entity->test2Map.ForEach([](auto& component){
					if (component.str == "hello") component.a -= 1;
					else component.a -= 4;
				});
			});
			TestEntity::test2MapComponents.ForEach([&result](auto entityIndex, auto& component){
				result += component.a;
			});
			
			if (result != 0) return -1;
			
			result = 2;
			TestEntity::ForEach([&result](auto entity){
				result -= entity->test2Map.Count();
			});
			
			if (result != 0) return -1;
			
			result = -1;
			TestEntity::ForEach([](auto entity){
				entity->test2Map.Erase("test1");
			});
			TestEntity::test2MapComponents.ForEach([&result](auto entityIndex, auto& component){
				result += component.a;
			});
			
			if (result != 0) return -2;
			
			TestEntity::ForEach([](auto entity){
				entity->Remove_test2Map();
			});
			
			TestEntity::test2MapComponents.ForEach([&result](auto entityIndex, auto& component){
				result += component.a;
			});
			
		}
		
		return result;
	}
}
