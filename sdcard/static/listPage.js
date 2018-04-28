var index_data;

$(function(){
    document.title = "メッセージ一覧 / " + document.title;
    
    $("#btn-write1,#btn-write2").on('click', write_list);
    $("#btn-new-msg1,#btn-new-msg2").on('click', new_msg);
    $("#btn-renum1,#btn-renum2").on('click', renum_msg);

    load_index({success: function(data, dataType){
        index_data = data;
        update_list(data)
        $(window).on('beforeunload',function(e){
            if(modified()){
                return "ちょっと待ってください。";
            }
        });
    }});
});

function modified() {
    var d = get_data();

    if(d.del_list.length > 0) return true;

    if(index_data.msg.length != d.msg.length) return true;
    
    var cnt = 0;
    for(let i = 0; i < index_data.msg.length; i++){
        for(let k in index_data.msg[i]){
            if( index_data.msg[i][k] != d.msg[i][k]){
                console.log("msg" + i + "." + k + " changed");
                cnt++;
            }
        }
    }
    return cnt > 0;
}

function update_list(data)
{
    let s = "<table class='table msg-list' border='0'>";
    s += "<tr>";
    s += "<th></th>";
    s += "<th class='col-enable'>有効</th>";
    s += "<th class='col-id'>id</th>";
    s += "<th class='col-type'>type</th>";
    s += "<th class='col-title'>タイトル</th>";
    s += "<th class='col-disp'>表示</th>";
    s += "<th class='col-edit'>編集</th>";
    s += "<th class='col-delete'>削除</th>";
    s += "</tr>";
    s += "<tbody id='msg-list-body'>";
    if(data.msg.length > 0){
        for(let msg of data.msg){
            s += make_row(msg);
        }
    } else {
        s += "<tr class='info'>";
        s += "<td colspan='8' class='col-no-msg'>";
        s += "メッセージが有りません。「新規メッセージ」で作成して下さい";
        s += "</td>";
        s += "</tr>";
    }
    s += "</tbody>";
    s += "</table>";

    $("#msg-table").html(s);
    $("#msg-list-body").sortable({
        axis: "y",
    });
}

function get_data()
{
    var tbody = $("#msg-list-body")[0];
    var msg = [];
    var del_list = [];
    for(let tr of tbody.children){
        if(tr.className.match('info')) continue;
        let id,enable,del;
        for(let td of tr.children){
            var cn = td.className;
            if(cn == "col-id"){
                id = $(td.children[0]).val();
            } else if(cn == "col-enable"){
                enable = $(td.children[0]).prop('checked');
            } else if(cn == "col-delete"){
                del = $(td.children[0]).prop('checked');
            }
        }
        if(del)
            del_list.push(id);
        else
            msg.push({id: id, enable: enable});
    }

    for(let i=0; i<msg.length; i++){
        let id = msg[i].id;
        let msg0 = index_data.msg;
        let j;
        for(j=0; j<msg0.length;j++){
            if(msg0[j].id == id) break;
        }
        if(j < msg0.length){
            msg[i].title = msg0[j].title;
            msg[i].player = msg0[j].player;
        }
    }
    return { msg: msg, del_list: del_list };
}
    
function write_list(){
    var d = get_data();

    if(d.del_list.length > 0){
        if(!confirm("Msg" + d.del_list.join(',') + "を削除しますか?"))
            return false;
        delete_msg(d.del_list);
    }

    write_index({msg: d.msg}, false);
    index_data.msg = d.msg;
    update_list(index_data);
}

function make_row(msg) {
    var s = "";
    
    s += "<tr>";
    s += "<td>";
    s += "<span class='ui-icon ui-icon-grip-solid-horizontal'></span>";
    s += "</td>";
    s += "<td class='col-enable' >"
        + "<input type='checkbox' class='form-control'";
    if(msg.enable)
        s += " checked";
    s += ">";
    s += "<td class='col-id'>";
    s += "<input type='text' value='";
    s += msg.id;
    s += "' class='form-control' size='1' disabled>"
    s += "</td>";
    s += "</td>";
    s += "<td class='col-type'>";
    s += msg.player;
    s += "</td>";
    s += "<td class='col-title'>"
        + "<input type='text' disabled size='30'"
        + " class='form-control' value='"
        + (msg.title || "")
        + "'>"
        + "</td>";
    s += "<td class='col-disp'>";
    s += "<a onClick='disp_msg(";
    s += msg.id;
    s += ")'><span class='ui-icon ui-icon-play'></span></a>";
    s += "</td>";
    s += "<td class='col-edit'>";
    s += "<a href='/edit?id=";
    s += msg.id;
    s += "'><span class='ui-icon ui-icon-pencil'></span></a>";
    s += "</td>";
    s += "<td class='col-delete'>"
    s += "<input type='checkbox' class='form-control'>";
    s += "</td>"
    s += "</tr>";
    return s;
}

function new_msg()
{
    var ids  = $("#msg-list-body .col-id");

    let id = 0;
    for(let td of ids){
        let id0 = $(td.children[0]).val();
        id = Math.max(id, id0);
    }
    id++;

    var d = get_data();
    var index = {msg: d.msg};

    make_new_msg(index, id);
    update_list(index);
    index_data = index;
}

function delete_msg(id_list)
{
    $.ajax({
        type: "get",
        url: "/api",
        dataType: "text",
        data: { cmd : "delete_msg", id : id_list.join() },
        success: function(data, dataType){
            status("delete_msg succeeded");
        },
        error: function(xhr, status, e){
            console.log(status + ":" + xhr.responseText);
        }
    });
}

function renum_msg()
{
    if(!confirm("メッセージのidを表示順に付け替えます。いいですか？"))
        return;
    $.ajax({
        type: "get",
        url: "/api",
        dataType: "json",
        data: {cmd : "renum_msg"},
        success: function(data, dataType){
            index_data = data;
            update_list(data)
            status("renum_msg succeeded");
        },
        error: function(xhr, status, e){
            console.log(status + ":" + xhr.responseText);
        }
    });
}
