'use strict';

var BaseObject = require('../hakurei').Object.Container;
var Util = require('../hakurei').Util;
var Card = require('./card');
var CONSTANT = require('../constant');

// 残りカードのボードの表示位置
var BOARD_X = 90;
var BOARD_Y = 50;

var Deck = function(scene) {
	BaseObject.apply(this, arguments);

	this._top_card = null;
};
Util.inherit(Deck, BaseObject);

Deck.prototype.init = function(){
	BaseObject.prototype.init.apply(this, arguments);

	this._top_card = null;

	this._generateCard();
	this._setTopCard();
};

// デッキ内のカード生成
Deck.prototype._generateCard = function(){
	this.removeAllObject();
	for (var i = 1; i <= 9; i++) { // number
		for (var j = 0; j <= 3; j++) { // type
			var card = new Card(this);
			card.init();
			card.setType(j, i);
			this.addObject(card);
		}
	}
};

// デッキトップのカードを決める
Deck.prototype._setTopCard = function(){
	var count = this.count();

	// デッキにもうカードがなければデッキトップも空
	if (count === 0) {
		this._top_card = null;
		return;
	}

	var i = Util.getRandomInt(count) - 1;

	var id = Object.keys(this.objects)[i];

	this._top_card = this.get(id);
};

Deck.prototype.beforeDraw = function(){
	BaseObject.prototype.beforeDraw.apply(this, arguments);
};

// 残りカードのボードを表示
Deck.prototype.draw = function(){
	var ctx = this.core.ctx;
	ctx.save();
	var objects = Util.shallowCopyHash(this.objects);

	for (var id in objects) {
		var card = objects[id];
		var x = BOARD_X;
		var y = BOARD_Y;
		if (card.type() === CONSTANT.TYPE_RED) {
			ctx.fillStyle = "red";
			y += 0;
		}
		else if (card.type() === CONSTANT.TYPE_BLUE) {
			ctx.fillStyle = "blue";
			y += 30;
		}
		else if (card.type() === CONSTANT.TYPE_GREEN) {
			ctx.fillStyle = "green";
			y += 60;
		}
		else if (card.type() === CONSTANT.TYPE_PURPLE) {
			ctx.fillStyle = "purple";
			y += 90;
		}

		x += card.number() * 20;

		ctx.fillRect(x, y, 20, 30);

		ctx.strokeStyle = "white";
		ctx.beginPath();
		ctx.moveTo(x, y);
		ctx.lineTo(x+20, y);
		ctx.lineTo(x+20, y+30);
		ctx.lineTo(x, y+30);
		ctx.closePath();
		ctx.stroke();

		ctx.fillStyle = "white";
		ctx.font = "18px 'MyFont'";
		ctx.textAlign = 'center';
		ctx.textBaseline = 'middle';

		ctx.fillText(card.number(), x + 10, y + 15);
	}
	ctx.restore();
};

// デッキトップのカードを返し、新しいカードをデッキトップにする
Deck.prototype.serve = function(){
	var ret = this.removeObject(this._top_card);

	// 表にする
	if (ret.isReverse()) {
		ret.flip();
	}

	this._setTopCard();

	return ret;
};

Deck.prototype.topCard = function(){
	return this._top_card;
};


module.exports = Deck;
