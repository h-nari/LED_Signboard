    <h1>設定</h1>

    <form class="form-horizontal">

      <div class="form-group">
        <label class="control-label col-xs-4">LEDモジュール・タイプ:</label>
        <div class="col-xs-4">
          <select id="config-moduleType" class="form-control">
            <option value="64x32s16" selected>
              64ドット x 32ドット (scan16)
             </option>
            <option value="32x32s16" selected>
              32ドット x 32ドット (scan16)
             </option>
            <option value="32x16s8" selected>
              32ドット x 16ドット (scan8)
             </option>
          </select>
        </div>
      </div>

      <div class="form-group">
        <label class="control-label col-xs-4">モジュール 横 個数:</label>
        <div class="col-xs-2">
          <input type="number" size="3" id="config-moduleColumn"
		 class="form-control" min="1">
        </div>
      </div>

      <div class="form-group">
        <label class="control-label col-xs-4">モジュール 縦 個数:</label>
        <div class="col-xs-2">
          <input type="number" id="config-moduleRow" class="form-control"
                 min="1">
        </div>
      </div>

      <div class="form-group">
        <label class="control-label col-xs-4">表示プレーン数:</label>
        <span class="col-xs-2">
          <input type="number" id="config-plane" class="form-control"
                 min="1" max="8">
        </span>
	<span class="explain">
	  1～8, プレーン数が増えると表示できる中間調が増えます
	</span>
      </div>

      <div class="form-group">
        <label class="control-label col-xs-4">明るさ:</label>
        <span class="col-xs-2">
          <input type="number" id="config-bright" class="form-control"
                 min="0" max="100">
        </span>
	<span class="explain">
	  100(最大輝度)～0(消灯)
	</span>
      </div>

      <div class="form-group">
        <label class="control-label col-xs-4">スクリプト:</label>
        <span class="col-xs-2">
	  <select id="config-script" class="form-control">
	    <option>スクリプト</option>
	  </select>
        </span>
	<button class="btn" id="btn-new-script">新規</button>
      </div>

      <div class="form-group">
        <label class="control-label col-xs-4">LEDモード (bit配列):</label>
        <span class="col-xs-4">
          <select id="config-ledMode" class="form-control">
            <option value="0" selected>
              モード0: (R0,G0,B0,R1,G1,B1)
             </option>
            <option value="1" selected>
              モード1: (G0,B0,R0,G1,B1,R1)
             </option>
          </select>
        </span>
        <span class="explain">
          色が変だったら、ここを変更してみて下さい
        </span>
      </div>

      <div class="form-group" class="part-block">
        <div class="col-xs-offset-2 col-xs-10">
          <button type="button" id="btn-write" class="btn btn-primary">書込</button>
        </div>
      </div>
    </form>
    <div id="new-script-dialog" title="新規スクリプト" style="display:none;">
    <form>
      <fieldset>
	<label for="name">スクリプト名:</label>
	<input type="text" name="name" placeholder="スクリプト名を入力"
	       id="config-script-name"
	       class="text ui-widget-content ui-cornaer-all">
      </fieldset>
    </form>
  </div>
  <script src="/confPage.js">
  </script>  
