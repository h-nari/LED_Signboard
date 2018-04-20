var msg_id;
var loaded_data;
var loaded_text = "";

$(function(){
    document.title = "メッセージ編集 / " + document.title;
    
    $("#overlay-select").change(select_overlay);
    $("#btn-write1,#btn-write2").on("click",writeMsg);
    $("#btn-disp1,#btn-disp2").on("click", function(){
        writeMsg();
        disp_msg(msg_id);
    });
    $("#btn-prev-msg").on("click", function(){ edit_next_msg(-1);}); 
    $("#btn-next-msg").on("click", function(){ edit_next_msg( 1);});
    $("#btn-title-set").on("click", titleSet);
    $("#btn-new-msg").on("click", new_msg);
    select_overlay();
    var params = getParams();

    if(params) {
        msg_id = params.id[0];
        $("#msg-id").val(msg_id);
        load_msg_file();
    }

    $(window).on('beforeunload',function(e){
        if(modified()){
            return "ちょっと待ってください。"
        }
    });
    load_font_list();
});

function modified() {
    var d = getMsgData();
    var cnt = 0;
    for(let k in loaded_data){
        if(loaded_data[k] != d[k]){
            console.log(k + " changed");
            cnt++;
        } 
    }

    if(d.player == "scroll-text"){
        var text = $("#scroll-text-text").val();
        if(loaded_text != text){
            console.log("text changed");
            cnt++;
        }
    }
    return cnt > 0;
}

function getParams(){
    "use strict";
    var query = location.search.substr(1);
    if(query.length < 1) return null;
    var params = {};
    query.split("&").forEach(function(item){
        var s = item.split("=");
        var k = decodeURIComponent(s[0]);
        var v = decodeURIComponent(s[1]);
        if(k in params) params[k].push(v);
        else params[k] = [v];
    });
    return params;
}

function load_msg_file()
{
    $.ajax({
        type: "get",
        url: "/api",
        dataType: "json",
        data: {cmd : "read_msg", id: msg_id },
        success : function(data, dataType){
            loaded_data = data;
            var player = data.player;
            $("#msg-title").val(data.title || "");
            $("#overlay-select").val(player);
            select_overlay();

            for(var prop in data){
                if(prop == "title" || prop == "player") continue;
                let name1 = "#msg-" + prop;
                let name2 = "#" + player + "-" + prop;
                let name = name1 + "," + name2;
                let type = $(name).prop("type");

                if(type == "checkbox"){
                    $(name).prop("checked", data[prop]);
                } else {
                    $(name).val(data[prop]);
                } 
            }
            
            load_font_list( data.font );
            if(player == "scroll-text"){
                load_txt_file();
            }
            
        },
        error: function(XMLHttpRequest, textStatus, e){
            console.log("error:" + textStatus + ":" + e.message);
        },
    });
}

function load_txt_file()
{
    status("load_txt_file");
    $.ajax({
        type: "get",
        url: "/api",
        dataType: "text",
        data: {cmd : "read_txt", id: msg_id},
        success : function(data, dataType){
            status("load_txt_file succeeded");
            var nl = data.match(/\n/g);
            var lc = nl ? nl.length : 0;
            var rows = Math.min(20,lc);

            loaded_text = data;
            $("#scroll-text-text").val(data);
            $("#scroll-text-text").attr("rows", rows);
        },
        error: function(XMLHttpRequest, textStatus, e){
            status("no htm file for id="+ msg_id + " :"
                   + textStatus + ":" + e.message);
        },
    });
}

function select_overlay(){
    var opt = $("#overlay-select option:selected");
    var target = opt.attr("target");
    var parts = $("#editor-overlay-part > div");
    for(let page of parts){
        if(page.id == target) $(page).show();
        else $(page).hide();
    }
}

function getMsgData()
{
    var id = $("#msg-id").val();
    var player = $("#overlay-select").val();
    var d = { player: player};
    
    for(let n of ['title','rx','ry','rw','rh','bg','pre-clear',
                  'post-clear']){
        let name = "#msg-" + n;
        let type = $(name).prop("type");
        if(type == "checkbox") {
            d[n] = $(name).prop("checked");
        } else {
            d[n] = $(name).val();
        } 
    }

    var part = "#"+player+"-player-part";
    var items = $(part + " input," +
                  part + " select");
    var prefix_len = player.length + 1;
    for(let item of items){
        let name = item.id.substring(prefix_len);
        if(item.tagName == "INPUT" && $(item).prop("type") == "checkbox")
            d[name] = $(item).prop("checked");
        else
            d[name] = $(item).val();
    }
    return d;
}
    
function writeMsg()
{
    var d = getMsgData();
    loaded_data = d;
    
    upload_file(msg_id, 'msg', d);
    if( d.player == "scroll-text"){
        loaded_text = $("#scroll-text-text").val();
        upload_file(msg_id, 'txt', loaded_text);
    }

    update_index(msg_id, d.player, $("#msg-title").val());
}

function update_index(id, player, title){
    $.ajax({
        type: "get",
        url: "/api" ,
        dataType: "json",
        data: {cmd: "read_index"},
        success: function(data, dataType){
            var msg = data.msg;
            for(let i=0; i<msg.length;i++){
                if(msg[i].id == id){
                    data.msg[i].player = player;
                    data.msg[i].title  = title;
                    write_index(data, false);
                    return;
                }
            }
            console.log("id:" + id + " not found in index file");
        },
        error: function(XMLHttpRequest, textStatus, e){
            console.log("fail to read index:" + textStatus + e.message);
            return;
        }
    });
}

function load_font_list(font = 0)
{
    $.ajax({
        type: "get",
        url: "/api",
        dataType: "json",
        data: {cmd: "font_list"},
        success: function(data, dataType){
            var s = "";
            for(let i=0; i<data.length; i++){
                s += "<option value='";
                s += i;
                s += "'";
                if(i == font)
                    s += " selected";
                s += ">";
                s += data[i];
                s += "</option>";
            }
            $("#static-text-font,#scroll-text-font").html(s);
        },
        error: function(XMLHttpRequest, textStatus, e){
            console.log("fail to get font_list:" + textStatus + e.message);
            return;
        }
    });
}

function edit_next_msg( delta )
{
    if(modified()){
        if(!confirm("データが変更されています。\n" +
                    "書込後、移動しますか？"))
            return;
        writeMsg();
    }
    
    load_index(function(data, dataType){
        for(let i=0; i< data.msg.length; i++){
            var m = data.msg[i];
            if(m.id == msg_id){
                if(i + delta < 0){
                    alert("MSG" + msg_id + "は先頭のメッセージです");
                    return;
                }
                if(i + delta >= data.msg.length){
                    alert("MSG" + msg_id + "は最後のメッセージです");
                    return;
                }
                var next_id = data.msg[i+delta].id;
                location.href = "/edit?id=" + next_id;
                return;
            }
        }
        alert("MSG" + msg_id + "が見つかりません");
    });
}


function titleSet()
{
    var player = $("#overlay-select").val();
    var text = "";
    
    if(player == "static-text" || player == "scroll-text"){
        text = $("#" + player + "-text").val();
        $("#msg-title").val(text.substring(0,16));
    }
}

function new_msg()
{
    if(modified()){
        if(!confirm("データが変更されています。\n" +
                    "書込後、新規メッセージを作成しますか？"))
            return;
        writeMsg();
    }
    
    load_index(function(data, dataType){
        var id = 0;
        for(let m of data.msg)
            id = Math.max(id, m.id);
        id++;
        make_new_msg(data, id);
        location.href = "/edit?id=" + id; 
    });
}
