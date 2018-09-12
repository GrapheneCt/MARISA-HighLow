'use strict';
var UIParts = require('../hakurei').Object.UIParts;

var GenerateButton = {};

GenerateButton.exec = function (scene) {
	var _pass_button = new UIParts(scene,  75, 480, 180 * 0.5, 30 * 1.5, _buttonDrawer("Pass"));
	var _high_button = new UIParts(scene, 190, 480, 180 * 0.5, 30 * 1.5, _buttonDrawer("High"));
	var _low_button  = new UIParts(scene, 290, 480, 180 * 0.5, 30 * 1.5, _buttonDrawer("Low"));
	var _same_button = new UIParts(scene, 390, 480, 180 * 0.5, 30 * 1.5, _buttonDrawer("Same"));

	return {
		pass_button: _pass_button,
		high_button: _high_button,
		low_button: _low_button,
		same_button: _same_button,
	};
};
function _buttonDrawer (text) {
	return function() {
		var ctx = this.core.ctx;
		var logo;
		if (this.onmouse) {
			logo = this.core.image_loader.getImage("button_gray");
		}
		else {
			logo = this.core.image_loader.getImage("button_white");
		}

		var offset_x = 0;
		var offset_y = 0;
		if (this.isclick) {
			offset_x = 3;
			offset_y = 3;
		}

		ctx.save();
		ctx.drawImage(logo,
			this.getCollisionLeftX() + offset_x,
			this.getCollisionUpY() + offset_y,
			logo.width * 0.5,
			logo.height * 1.5
		);
		ctx.textAlign = 'center';
		ctx.textBaseline = 'middle';
		ctx.font = "24px 'MyFont'";
		ctx.fillStyle = 'black';

		ctx.fillText(text, this.x() + offset_x, this.y() + offset_y);

		ctx.restore();
	};
}

module.exports = GenerateButton;