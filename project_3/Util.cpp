//
//  Util.c
//  hw4
//
//  Created by Justin Hust on 10/18/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

int random_between(int min, int max) {
  return rand() % (max-min) + min;  
}
