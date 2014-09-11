4 1d

_HT__info
2 2
wc(ia64lin/gcc)
little_endian

_HT__base_types
8 2
char
short
int
long
longlong
float
double
void *

_HT__user_types
1 3
stat

stat
12 3
st_dev
st_ino
st_nlink
st_mode
st_uid
st_gid
pad0
st_rdev
st_size
st_atime
__reserved0
st_mtime
__reserved1
st_ctime
__reserved2
st_blksize
st_blocks
__unused


_HT__info
2 2
gen_CtoP
O 1 -
I

value
O 1 -
I

1

0 1
Ie8

1

1 1
I1


_HT__base_types
8 2
size
O 1 -
I

align
O 1 -
I

2

0 1
I8

1 1
I8

2

0 1
I10

1 1
I10

2

0 1
I20

1 1
I20

2

0 1
I40

1 1
I40

2

0 1
I40

1 1
I40

2

0 1
I20

1 1
I20

2

0 1
I40

1 1
I40

2

0 1
I40

1 1
I40


_HT__user_types
1 3
size
O 1 -
I

align
O 1 -
I

union
O 1 -
I

3

0 1
I480

1 1
I40

2 1
I0


stat
12 3
decl
O 1 -
S

offset
O 1 -
I

size
O 1 -
I

3

0 1
Sunsigned long st_dev

1 1
I0

2 1
I40

3

0 1
Sunsigned long st_ino

1 1
I40

2 1
I40

3

0 1
Sunsigned long st_nlink

1 1
I80

2 1
I40

3

0 1
Sunsigned int st_mode

1 1
Ic0

2 1
I20

3

0 1
Sunsigned int st_uid

1 1
Ie0

2 1
I20

3

0 1
Sunsigned int st_gid

1 1
I100

2 1
I20

3

0 1
Sint pad0

1 1
I120

2 1
I20

3

0 1
Sunsigned long st_rdev

1 1
I140

2 1
I40

3

0 1
Slong st_size

1 1
I180

2 1
I40

3

0 1
Slong st_atime

1 1
I1c0

2 1
I40

3

0 1
Slong __reserved0

1 1
I200

2 1
I40

3

0 1
Slong st_mtime

1 1
I240

2 1
I40

3

0 1
Slong __reserved1

1 1
I280

2 1
I40

3

0 1
Slong st_ctime

1 1
I2c0

2 1
I40

3

0 1
Slong __reserved2

1 1
I300

2 1
I40

3

0 1
Slong st_blksize

1 1
I340

2 1
I40

3

0 1
Slong st_blocks

1 1
I380

2 1
I40

3

0 1
Slong __unused[3]

1 1
I3c0

2 1
Ic0



