Copyright 2016 Sony Digital Network Applications, Inc.
All Rights Reserved.
--------------------------------------------------------------------------------

SmartAR SDK

--------------------------------------------------------------------------------
SmartAR(TM) SDKはマルチプラットフォーム対応のAR(Augmented Reality：拡張現実感)
アプリケーション開発環境です。

開発者は本SDKが提供するAPI群を使用することにより、拡張現実を実現する技術である
SmartAR(TM)を使用したアプリケーションを容易に開発することができます。

--------------------------------------------------------------------------------
0. 目次

  1. リリースパッケージ構成
  2. ポータルサイト
  3. 問い合わせ先

--------------------------------------------------------------------------------
1. リリースパッケージ構成

  SmartAR SDKのパッケージ構成は下記となります。


  SmartAR_SDK_Android
   |
   +-- README_Android.txt  ...........................................(1)
   |
   +-- ReleaseNote_Android.txt  ......................................(2)
   |
   +-- License.txt  ..................................................(3)
   |
   +-- Document  .....................................................(4)
   |     +-- SmartARSDK-Overview_j/e.pdf
   |     +-- SmartARSDK-Reference_j/e.pdf
   |     +-- logo
   |
   +-- SmartAR  ......................................................(5)
   |     +-- lib
   |     +-- include
   |
   +-- Sample  .......................................................(6)
         +-- android
         |     +-- sample_api_***
         |     +-- target_picture
         |     +—  AndroidStudioPatch
         |     +-- README_sample_Android.txt
         |
         +-- mobile_common


  --------------------------------------------------------------------------
  (1)README_Android.txt
    本ファイルです。

  (2)ReleaseNote_Android.txt
    リリースノートです。

  (3)License.txt
    SmartARで利用しているBSDライセンスの情報です。
    リリースの際の使用許諾契約書(EULA)への表記には必ず含めてください。

  (4)Document
    SmartARの概要、API仕様書、ロゴのガイドライン等が格納されています。

    ・SmartARSDK-Overview_j/e.pdf
      SmartAR SDKの概要、使用方法、参考情報が記載されています。

    ・SmartARSDK-Reference_j/e.pdf
      SmartAR SDKのAPI仕様書です。

    ・logo
      SmartARのロゴに関するガイドライン、画像データが格納されています。

  (5)SmartAR
    SmartARのライブラリ 及び インクルードファイルが格納されています。
    使用方法/使用例については、API仕様書とSampleアプリをご参照ください。

  (6)Sample
    SmartAR SDKを利用したサンプルアプリが格納されています。
    サンプルを動作させる場合、android, mobile_commonディレクトリ以下の
    ファイル構成は変更しないでください。

    ・android/sample_api_***
      SmartAR APIを利用したサンプルアプリ プロジェクトです。
      サンプルアプリ プロジェクトのビルド環境情報については、
      README_sample_Android.txt をご参照ください。

    ・android/target_picture
      SmartARのサンプルアプリで使用する認識ターゲット画像が格納されています。

    ・android/AndroidStudioPatch
      Android Studioでサンプリアプリをビルドするためのファイルです。
      ビルド方法については README_sample_Android.txt をご参照ください。

    ・mobile_common
      サンプルアプリ プロジェクトで利用するサンプルコードの一部です。
      Android/iOSのマルチプラットフォーム向けにNativeで実装されています。
      サンプルを動作させるために必要なファイルです。

--------------------------------------------------------------------------------
2．ポータルサイト
   リリース情報やFAQ等の情報を掲載しています。
   http://www.cybernet.co.jp/ar-vr/products/smartar/

--------------------------------------------------------------------------------
3. 問い合わせ先
   ご質問等は以下までお願いいたします。
   <smartar-info@cybernet.co.jp>

----------------------------------------------------------------------------[EOF]