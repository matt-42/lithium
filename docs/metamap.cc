// __documentation_starts_here__

/*
# metamap


## Introduction 

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

In C or C++ it is harder, you need to declare the structure of your object before instantiating it,
and you simply can't iterate over the members of an object:

```c++
struct Person {
  std::string name;
  int age;
};

Person person{"John", 42};

std::cout << "name: " << person.name << std::endl;
std::cout << "age: " << person.age << std::endl;
```

To solves the declaration problem `metamap` provides a way to declare and instance plain 
C++ objects in just one C++ statement:

*/
auto person = mmm(s::name = "John", s::age = 42);
// Note: mmm is a shortcut for make_metamap.
assert(person.name == "John");
assert(person.age == 42);
/*
[More info on `s::xxxx` symbols](#getting-started/automatic-symbols-generation).

And it solves the member iteration problem by providing a way to map a function of each object members:
*/
map(person, [] (auto key, auto value) { 
  std::cout << symbol_string(key) << " : " << value << std::endl; 
});
/*
`map` is unrolled at compile time so its has no runtime cost.

## Algorithms

On top of this, `metamap` provides some algorithms:

- `cat`: Concatenation of two metamaps. Values of m1 are given the priority in case of dupplicate keys.

*/
auto m3 = cat(m1, m2);
/*

- `intersection`: Build the map containing keys present in m1 and m2, taking values from m1.

*/
auto m4 = intersection(m1, m2);
/*

- `substract`: Build the map containing keys present in m1 but not in m2, taking values from m1.

*/
auto m5 = substract(m1, m2);
