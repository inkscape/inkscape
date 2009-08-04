#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
from scour import scourString
input = file(sys.argv[1], "r")
sys.stdout.write(scourString(input.read()).encode("UTF-8"))
input.close()
sys.stdout.close()
