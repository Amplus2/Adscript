SRC_DIR = src

default:
	make -C $(SRC_DIR)

test:
	make -C $(SRC_DIR) test
