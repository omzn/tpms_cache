# BLE TPMS cache

* BLEでタイヤの空気圧センサーの値を飛ばすTPMSを受信して，同じ値をadvertiseするBLEキャッシュサーバ．
* 車内で受信するシステムをESP-WROOM-32を用いて作ったが，受信感度が弱く，TPMSの電波を直に拾いづらい．
* M5Stackのようなしっかりしたアンテナを積んだもので一旦TPMSを受信して，その内容を車内に配信．
* 結果的に受信側は1つのデバイスだけを監視すれば良くなるので，処理的にも軽くなった．

* M5Stackを推奨．ただし，車載する際にはLiPoは外した方が良い．
