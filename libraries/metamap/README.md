li::metamap
===============================

In dynamic languages, instanciating an object is simple, for example in javascript:

```js
var person = { name: "John", age: 42 };
```

Javascript also provide a way to iterate on the object member:
```js
for(var key in person){
    console.log(key + ': ' + person[key]);
}
```

In C++ it is harder, you need to declare the structure of your object before instantiating it,
and you simply can't iterate over the properties of an object.

Thanks to this library, you can reach Javascript simplicity without loosing the performances of C++:
```c++
auto person = mmm(s::name = "John", s::age = 42); // mmm means Make MetaMap

map(person, [] (auto key, auto value) { std::cout << symbol_string(key) << value << std::endl; });
```

Everything is static, no hashmap or other dynamic structure is built by li::metamap. You can
view it as compile-time introspection.
As a comparison, this code is equivalent (in terms of runtime) to the less generic:
```c++
struct { const char* name, int age } person{"John", 42};
std::cout << "name" << person.name << std::endl;
std::cout << "age" << person.age << std::endl;
```

On top of this, li::metamap provides some algorithms that where only possible
on dynamic structures before:


- li::cat: Concatenation of two maps. Values of m1 are given the priority in case of dupplicate keys.

```c++
auto m3 = li::cat(m1, m2);
```

- li::intersection: Build the map containing keys present in m1 and m2, taking values from m1.

```c++
auto m4 = li::intersection(m1, m2);
```

- li::substract: Build the map containing keys present in m1 but not in m2, taking values from m1.

```c++
auto m5 = li::substract(m1, m2);
```
