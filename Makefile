# Makefile for BlueDOG programming language

CC = gcc
CFLAGS = -Wall -g  # コンパイルオプション（デバッグ情報を含む）
TARGET = bluedog
SRC = bluedog.c  # ソースファイル

# インストール先ディレクトリ
PREFIX = /usr/local/bin

# デフォルトのターゲット（makeコマンドで最初に実行される）
all: $(TARGET)

# bluedogをコンパイル
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# インストール
install: $(TARGET)
	cp $(TARGET) $(PREFIX)

# クリーン
clean:
	rm -f $(TARGET)

# プログラムの実行
run: $(TARGET)
	./$(TARGET) program.bld
