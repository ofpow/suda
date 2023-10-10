# Suda
Super Ultimate Deluxe Awesome

A small dynamic language written in C

## Syntax

Hello world:
```python
print "hello world\n"
```

Variables:
```python
# 3 types: number, string and array
let x = 3
let y = "hello"
let z = [1, 2, 3, "hello", "world"]
```

If statements:
```python
if 1
    print "hello!\n"
else
    print "goodbye\n"
;
```

While loops:
```python
let count = 1
while (count < 10)
    print count
    count = (count + 1)
;
```

Functions:
```python
fn foo(x)
    print x
    return x
;

print foo(3)
```

## Usage
```shell
cc -o suda suda.c
./suda [script]
```
