# gzip_demo
libdeflateを使ってgzipファイルの展開を実装してみました。
※圧縮前のファイルが4GB以上の場合は正常に動作しません。
※メモリの使いすぎを防止するため、圧縮後のファイルサイズが1.5GBより大きい場合は動作しないようになっています。

## コンパイル
事前にlibdeflate-dev (UbuntuやDebianの場合)のインストールが必要です。
```
$ make
```

## テスト
```
$ make test
```

## 実行
```
$ ./gunzip_demo input output
```
