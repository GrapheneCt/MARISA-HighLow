'use strict';

var BaseScene = require('./base');
var Util = require('../../hakurei').Util;
var CONSTANT = require('../../constant');

var SceneDuelPass = function(core) {
	BaseScene.apply(this, arguments);
};
Util.inherit(SceneDuelPass, BaseScene);

SceneDuelPass.prototype.init = function(){
	BaseScene.prototype.init.apply(this, arguments);

	// トップを表に
	this.parent.deck().topCard().flip();
};


SceneDuelPass.prototype.beforeDraw = function(){
	BaseScene.prototype.beforeDraw.apply(this, arguments);

	// N秒間は表にしたカードをその場所で見せ続ける
	if(this.frame_count < 60) {
		return;
	}
	else if (this.frame_count === 60) {
		this.parent.startSerifExtinguish();

		// クリア判定
		if (this.parent.rule_manager.isClear()) {
			this.core.scene_manager.setFadeOut(60, "black");
			this.core.scene_manager.changeScene("clear");
		}

		return;
	}
	else if (this.parent.rule_manager.isClear()) {
		// クリア済みならシーンが変わるまで何もしない
	}
	else {
		// 左のカードを右へ移動する演出
		var x = this.parent.deck().topCard().x() + 10;
		this.parent.deck().topCard().x(x);

		// 移動が終わったら
		if (x >= CONSTANT.OPEN_CARD_X) {
			this.parent.setNewCard();

			if (this.parent.rule_manager.isGameOver()) {
				// ゲームオーバー
				this.parent.changeSubScene("not_reach");
			}
			else {
				// 次へ
				this.parent.changeSubScene("choose");
			}
		}
	}
};

SceneDuelPass.prototype.draw = function(){
	BaseScene.prototype.draw.apply(this, arguments);
};

module.exports = SceneDuelPass;
