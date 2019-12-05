li::metamap
===============================


# Introduction 

In dynamic languages, instanciating an object is simple, for example in javascript:

```js
var person = { name: "John", age: 42 };
```

Javascript also provide a way to iterate on the object members:
```js
for(var key in person){
    console.log(key + ': ' + person[key]);
}
```

In C++ it is harder, you need to declare the structure of your object before instantiating it,
and you simply can't iterate over the members of an object.

This library aims at providing a new C++ paradigm that enables you to reach Javascript simplicity 
without loosing the performances of C++:
```c++
// Declare an object
auto person = mmm(s::name = "John", s::age = 42); // mmm means Make MetaMap

// Iterate on the members.
map(person, [] (auto key, auto value) { std::cout << symbol_string(key) << value << std::endl; });

// You can also use it as a plain C++ object:
person.name = "Aurelia";
person.age = 52;

// Note s::name and s::age must be defined earlier with LI_SYMBOL(name); LI_SYMBOL(age);
```

Everything is static, no hashmap or other dynamic structure is built by li::metamap. You can
view it as compile-time introspection.
As a comparison, this code is equivalent in terms of runtime to the less generic:
```c++
struct { const char* name, int age } person{"John", 42};
std::cout << "name" << person.name << std::endl;
std::cout << "age" << person.age << std::endl;
```

On top of this, li::metamap provides some algorithms that where only possible
on dynamic structures before:


- `li::cat`: Concatenation of two maps. Values of m1 are given the priority in case of dupplicate keys.

```c++
auto m3 = li::cat(m1, m2);
```

- `li::intersection`: Build the map containing keys present in m1 and m2, taking values from m1.

```c++
auto m4 = li::intersection(m1, m2);
```

- `li::substract`: Build the map containing keys present in m1 but not in m2, taking values from m1.

```c++
auto m5 = li::substract(m1, m2);
```

# Applications

Most of the other libraries of the Lithium project are using this paradigm to build
zero-cost abstraction without the need of complex meta-programing. I invite you to check them out
to see how metamap can be used in concrete applications.


# Installation / Supported compilers

Everything explained here: https://github.com/matt-42/lithium#installation

# Authors

Matthieu Garrigues https://github.com/matt-42

# Donate

If you find this project helpful, please consider donating:
https://www.paypal.me/matthieugarrigues
