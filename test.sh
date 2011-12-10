#!/bin/bash

./cl < jp$1 execute > /tmp/out
diff sjp$1 /tmp/out
