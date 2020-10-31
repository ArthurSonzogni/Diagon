# Diagon

[![logo](./tools/logo.png)](https://arthursonzogni.com/Diagon/)

Diagon is an interactive interpreter. It transforms markdown-style expression
into an ascii-art representation.

It is written in C++ and use WebAssembly, HTML and CSS to make a Web
Application.

### [https://arthursonzogni.com/Diagon/](https://arthursonzogni.com/Diagon/)

**Table of content**
 * [Diagon](#diagon)
 * [Generators](#generators)
    * [Mathematic Expression](#mathematic-expression)
    * [Sequence Diagram](#sequence-diagram)
    * [Tree](#tree)
    * [Frame](#frame)
    * [Table](#table)
    * [Planar graph](#planar-graph)
 * [Command line interface](#command-line-interface)
 * [Thanks](#thanks)

# Generators

## Mathematic Expression

input:
~~~
f(x) = 1 + x / (1 + x)
~~~

output (Unicode):
~~~
             x  
f(x) = 1 + ─────
           1 + x
~~~

input:
~~~
sqrt(1+sqrt(1+x/2))
~~~

output (Unicode)
~~~
     _____________
    ╱        _____
   ╱        ╱    x
  ╱  1 +   ╱ 1 + ─
╲╱       ╲╱      2
~~~

input:
~~~
f(x) = 1 + x^2 + x^3 + x^(1+1/2)
~~~

output (Unicode):
~~~
                      ⎛    1⎞
                      ⎜1 + ─⎟
            2    3    ⎝    2⎠
f(x) = 1 + x  + x  + x       
~~~

input:
~~~
sum(i^2,i=0,n) = n^3/2+n^2/2+n/6
~~~

output (Unicode):
~~~
  n                   
 ___        3    2    
 ╲     2   n    n    n
 ╱    i  = ── + ── + ─
 ‾‾‾        2    2   6
i = 0                 
~~~

input:
~~~
int(x^2/2 * dx ,0,1) = 1/6
~~~

output (Unicode):
~~~
1            
⌠  2         
⎮ x         1
⎮ ── ⋅ dx = ─
⌡  2        6
0            
~~~

input:
~~~
[a;b] + [c;d] = [a+c; b+d]
~~~

output (Unicode):
~~~
⎛a⎞   ⎛c⎞   ⎛a + c⎞
⎜ ⎟ + ⎜ ⎟ = ⎜     ⎟
⎝b⎠   ⎝d⎠   ⎝b + d⎠
~~~

input:
~~~
[1,2;3,4] * [x;y] = [1*x+2*y; 3*x+4*y]
~~~

~~~
⎛1 2⎞   ⎛x⎞   ⎛1 ⋅ x + 2 ⋅ y⎞
⎜   ⎟ ⋅ ⎜ ⎟ = ⎜             ⎟
⎝3 4⎠   ⎝y⎠   ⎝3 ⋅ x + 4 ⋅ y⎠
~~~


## Sequence Diagram

input
~~~
Alice -> Bob: Hello Bob!
Alice <- Bob: Hello Alice!
~~~

Output (Unicode)
~~~
┌─────┐       ┌───┐
│Alice│       │Bob│
└──┬──┘       └─┬─┘
   │            │  
   │ Hello Bob! │  
   │───────────>│  
   │            │  
   │Hello Alice!│  
   │<───────────│  
┌──┴──┐       ┌─┴─┐
│Alice│       │Bob│
└─────┘       └───┘
~~~

Input
~~~
Renderer -> Browser: BeginNavigation()
Browser -> Network: URLRequest()
Browser <- Network: URLResponse()
Renderer <- Browser: CommitNavigation()
Renderer -> Browser: DidCommitNavigation()
~~~

Output (Unicode)
~~~
 ┌────────┐            ┌───────┐     ┌───────┐
 │Renderer│            │Browser│     │Network│
 └───┬────┘            └───┬───┘     └───┬───┘
     │                     │             │    
     │  BeginNavigation()  │             │    
     │────────────────────>│             │    
     │                     │             │    
     │                     │URLRequest() │    
     │                     │────────────>│    
     │                     │             │    
     │                     │URLResponse()│    
     │                     │<────────────│    
     │                     │             │    
     │ CommitNavigation()  │             │    
     │<────────────────────│             │    
     │                     │             │    
     │DidCommitNavigation()│             │    
     │────────────────────>│             │    
 ┌───┴────┐            ┌───┴───┐     ┌───┴───┐
 │Renderer│            │Browser│     │Network│
 └────────┘            └───────┘     └───────┘
~~~


Input
~~~
1) Renderer -> Browser: Message 1
2) Renderer <- Browser: Message 2

Renderer: 1<2
Browser: 2<1
~~~

Output (Unicode)
~~~
 ┌────────┐┌───────┐
 │Renderer││Browser│
 └───┬────┘└───┬───┘
     │         │    
     │──┐      │    
     │Message 2│    
     │<────────│    
     │  │      │    
     │Message 1│    
     │  └─────>│    
 ┌───┴────┐┌───┴───┐
 │Renderer││Browser│
 └────────┘└───────┘
~~~

## Tree

Input
~~~
Linux
  Android
  Debian
    Ubuntu
      Lubuntu
      Kubuntu
      Xubuntu
      Xubuntu
    Mint
  Centos
  Fedora
~~~

Output (Style Unicode 1)
~~~
Linux
 ├─Android
 ├─Debian
 │  ├─Ubuntu
 │  │  ├─Lubuntu
 │  │  ├─Kubuntu
 │  │  ├─Xubuntu
 │  │  └─Xubuntu
 │  └─Mint
 ├─Centos
 └─Fedora
~~~

Output (Style ASCII 2)
~~~
Linux
 +--Android
 +--Debian
 |   +--Ubuntu
 |   |   +--Lubuntu
 |   |   +--Kubuntu
 |   |   +--Xubuntu
 |   |   `--Xubuntu
 |   `--Mint
 +--Centos
 `--Fedora
~~~

Output (Style Unicode right top)
~~~
───Linux─┬─Android
         ├─Debian─┬─Ubuntu─┬─Lubuntu
         │        │        ├─Kubuntu
         │        │        ├─Xubuntu
         │        │        └─Xubuntu
         │        └─Mint
         ├─Centos
         └─Fedora
~~~

Output (Style Unicode right center)
~~~
         ┌─Android
         │                 ┌─Lubuntu
         │                 ├─Kubuntu
         ├─Debian─┬─Ubuntu─┼─Xubuntu
───Linux─┤        │        └─Xubuntu
         │        └─Mint
         ├─Centos
         └─Fedora
~~~


## Frame

Input
~~~
#include <iostream>
using namespace std;

int main() 
{
    cout << "Hello, World!";
    return 0;
}
~~~

Output
~~~
┌─┬────────────────────────────┐
│1│#include <iostream>         │
│2│using namespace std;        │
│3│                            │
│4│int main()                  │
│5│{                           │
│6│    cout << "Hello, World!";│
│7│    return 0;               │
│8│}                           │
└─┴────────────────────────────┘
~~~

## Table

Input
~~~
Column 1,Column 2,Column 3
C++,Web,Assembly
Javascript,CSS,HTML
~~~

Output (Style Unicode)
~~~
┌──────────┬────────┬────────┐
│Column 1  │Column 2│Column 3│
├──────────┼────────┼────────┤
│C++       │Web     │Assembly│
├──────────┼────────┼────────┤
│Javascript│CSS     │HTML    │
└──────────┴────────┴────────┘
~~~

## Planar graph

Input
~~~
if -> "then A" -> end
if -> "then B" -> end
end -> loop -> if
~~~

Output (Unicode)
~~~
┌──────────┐     
│    if    │     
└△─┬──────┬┘     
 │ │     ┌▽─────┐
 │ │     │then A│
 │ │     └┬─────┘
 │┌▽─────┐│      
 ││then B││      
 │└┬─────┘│      
 │┌▽──────▽─┐    
 ││   end   │    
 │└┬────────┘    
┌┴─▽─┐           
│loop│           
└────┘           
~~~

# Command line interface

Diagon is also usable as a command line tool.
For instance:
```bash
echo ""1+1/2 + sum(i,0,10) = 112/2" | diagon Math
            10         
          ___        
      1   ╲       112
  1 + ─ + ╱   i = ───
      2   ‾‾‾      2 
           0         
```
To use it, you can either compile it from source or use the precompiled binaries published on the snapstore:
~~~bash
sudo snap install diagon
~~~
[![snapstore](https://snapcraft.io/diagon/badge.svg)](https://snapcraft.io/diagon)
[![Get it from the Snap Store](https://snapcraft.io/static/images/badges/en/snap-store-white.svg)](https://snapcraft.io/diagon)  


# Thanks

This project has been possible thanks to these great projects
 * [ANTLR](http://www.antlr.org/)
 * [Boost-graph](https://www.boost.org/doc/libs/1_66_0/libs/graph/doc/)
 * [CMake](https://cmake.org/)
 * [Emscripten](https://github.com/kripken/emscripten)
 * [Git](https://git-scm.com/)
 * [KGT](https://github.com/katef/kgt)
 * [LLVM](https://llvm.org/)
 * [NeoVim](https://neovim.io/)
 * [PlantUML](http://plantuml.com/)
 * [WebAssembly](https://webassembly.org/)
