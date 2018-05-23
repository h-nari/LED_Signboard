# LED_Signboard
a arduino sketch for HSES-LMC1(esp8266) that run as a webserver and control LED Signboard

## 概要

<img src="http://www.humblesoft.com/products/HSES-LMC/photo/170628b9.jpg" width="320" align="right">

このプログラムは ESP8266を内蔵したWiFi LEDコントローラ HSES-LMC1と
LEDモジュールを組み合わせたLEDディスプレイを
電光掲示板として使用するためのプログラムです。
esp8266 arduinoでコンパイル＆書込を行います。

このプログラムを書き込むと、HSES-LMC1は
Webサーバとして動作し、ブラウザでアクセスすることで
表示するメッセージの入力、編集を行うことができます。

HSES-LMC1は無線LAN（WiFi)経由でネットワークに接続するので、
無線LANが使えない環境では、このプログラムは使用できません。

## 主な機能

- 静止文字の表示
- スクロール文字の表示
- ブラウザ上で表示内容を作成・編集
- ブラウザから表示の一時停止、再開、クリア等

## 対応するブラウザ

ブラウザは google chrome, FireFoxのみです。
他のブラウザ、Internet Explorer, Edge, Safari, Opera等では動作しません。

## ディレクトリ構成

各サブディレクトリの中身を紹介します。

- LED_Signboard: プログラム(スケッチ)
- files: javascriptやhtmlの元ファイル
- img: ドキュメント用の画像ファイル
- python: 開発用のpythonプログラム
- sdcard: sdcardに書き込むファイル


## 詳細な情報

- [Wiki](https://github.com/h-nari/LED_Signboard/wiki)
  - [必要なハードウェア](https://github.com/h-nari/LED_Signboard/wiki/Hardware)
  - [インストール方法](https://github.com/h-nari/LED_Signboard/wiki/Install)
  - [使用説明書](https://github.com/h-nari/LED_Signboard/wiki/Manual)

## 変更履歴

### 2018年5月23日(水)

- v 1.1
-- スクリプトの操作(zip upload/download, 削除、コピー)の追加
-- ビットマップメッセージ追加


### 2018年4月24日(火)

- v 1.0 リリース
