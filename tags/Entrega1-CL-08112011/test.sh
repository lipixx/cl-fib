#!/bin/bash

./cl < jp$1 > /tmp/out
diff sjp$1 /tmp/out
