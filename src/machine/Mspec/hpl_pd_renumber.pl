#!/usr/bin/perl

$count = 1000;

while(<>) {
  if(/^#define\s+PLAYDOHop_/) {
    s/\d{4}/$count/;
    $count++;
  }
  print;
}
