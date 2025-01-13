# gzip_demo
libdeflateを使ってgzipファイルの展開を実装してみました。
※圧縮前のファイルが4GB以上の場合は正常に動作しません。

## コンパイル
事前にlibdeflate-dev (UbuntuやDebianの場合)のインストールが必要です。
```
$ g++ -o gunzip_demo gunzip_demo.cpp gzipdata.cpp -ldeflate
```

## 実行
```
$ ./gunzip_demo input output
```
