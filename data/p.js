var local_available = true;
var remote_available = true;
var fetching_local = false;
var fetching_remote = false;

function onReady(yourMethod) {
  var readyStateCheckInterval = setInterval(function() {
    if (document && document.readyState === 'complete') { // Or 'interactive'
      clearInterval(readyStateCheckInterval);
      yourMethod();
    }
  }, 10);
}


function saveforms() {
// save the remote first, then the local
    if (remote_available == true) { 
        saveform( "remote", "/r900x_params_remote.txt" );
    }
    if (local_available == true) {
        if (remote_available == false) {
          saveform( "local", "/r900x_params.txt" );  
        }
        if (remote_available == true) {
          setTimeout(function() {
                saveform( "local", "/r900x_params.txt" );      
            }, 5000);
        }
    }
}

function display_message(mymsg) { 
    x = document.getElementById("mymsg");
    x.innerHTML = mymsg;
}

function saveform(tableid, tourl) {

  var xhttp = new XMLHttpRequest();

  document.getElementById("sub").innerHTML = /*tableid +*/ " Processing request, please Wait...";

  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("sub").innerHTML = /*tableid +*/ " Parameters saved.";
    }
    if (this.readyState == 4 && this.status == 201) {
      document.getElementById("sub").innerHTML = /*tableid +*/ " Parameters saved and activated.";
    }
    if (this.readyState == 4 && this.status == 202) {
      document.getElementById("sub").innerHTML = /*tableid +*/ " Parameters saved but not activated. Your modem might not be configured as expected";
    }
  };

  var payload = "";

    var table = document.getElementById(tableid);

    // skip ZERO'th row, as it's the TH header row.
    for (var i = 1, row; row = table.rows[i]; i++) {

       var id_name = row.cells[0].innerHTML; 

            stuff = id_name.split(":");
            stuff2 = stuff[1].split("=");

            //x:y=z
            id=stuff[0];
			name=stuff2[0];
			//value=stuff2[1];

			// id2 is the one we put in the payload as a string
			// id is the name we use for the html field
			id2 = id;

            if ( tourl == "/r900x_params_remote.txt" ) { 
                // add a  'R' to the start of the id 
                id = "R" + id; 
            } 
			
			
            if (id == "R&amp;E") { id = "R&E"; id2="&E"; } 
            if (id == "&amp;E")  { id = "&E"; id2="&E";} 

            f = document.getElementById(id2);

			
            if ( f == null ) continue; // skip invalid fields if not present
            value = document.getElementById(id).value
            payload = payload + id2+":"+name+"="+value + "\r\n";
        
    }

  var url = "/psave?f="+tourl; 

  xhttp.open("PUT", url, true);
  xhttp.send(payload); 

}

onReady(function() { loadforms(); } );

function loadforms( ) {
    loadform( "local", "/r900x_params.txt" );
    loadform( "remote", "/r900x_params_remote.txt" );
}

function reload_fresh_params() { 

    //document.getElementById("fresh").value = "Talking With Radio/s, please wait...( page will reload )";

    display_message(" Fetching radio parameters. Please, wait.");

    unloadform('remote');
    reload_fresh("/prefresh?type=remote",false);

    fetching_remote = true;
    loadform( "remote", "/r900x_params_remote.txt" );

    unloadform('local');
    reload_fresh("/prefresh?type=local",false);
    fetching_local = true;
    loadform( "local", "/r900x_params.txt" );

    document.getElementById("fresh").value = "Load fresh parameters";

    //display_message("");

} 

// same as a reload_fresh_params(), but with an additional factory=yes, which behind the curtain adds an AT&W and AT&F etc 
// just before reading fresh params from the radio.
function factory() { 
  unloadforms();
  document.getElementById("fresh").value = "Performing factory reset. The page will reload automatically. Please, wait.";
  reload_fresh("/prefresh?type=remote&factory=yes",false);
  reload_fresh("/prefresh?type=local&factory=yes",false);
  loadforms();
  document.getElementById("fresh").value = "Load fresh parameters";
} 

function reload_fresh(url,reload) { 

  var xhttp = new XMLHttpRequest();
  var jsondata = '';

  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      jsondata = this.responseText;

        var obj = JSON.parse(jsondata);

        //if obj.request == "paramrefresh" 
        //if obj.type == "local" or "remote"
        //if obj.num_read == 23

        var displayable = obj.num_read-1;

        var user_msg = "Refreshed ("+displayable+") parameters from the ("+obj.type+") radio";
        if  (( displayable == 0 )  && ( obj.type == "remote" ) ) { 
            user_msg += "\nCannot communicate with your remote radio, \nPlease check Settings using AT command set.";
        }

        //alert(user_msg);

        if ( reload) {
            window.location.reload(true);
           
        } 
    }
    if (this.readyState == 4 && this.status == 404) {
        alert("err, unhandled:"+url);
    }

  }

  xhttp.open("GET", url, true);
  xhttp.send();

} 

function unloadforms(){
     unloadform('remote');
     unloadform('local');
}
function unloadform(tableid){ 

    var table = document.getElementById(tableid);
    if ( tableid == 'local' ) {
    table.innerHTML = '';
    }
    if ( tableid == 'remote' ) {
    table.innerHTML = '';
    }
} 

function loadform( tableid, fromurl ) {

  var xhttp = new XMLHttpRequest();
  var paramdata = '';

  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      paramdata = this.responseText;
        var paramsarray = paramdata.split("\n");

        // iterate over the paramdata string, breakup and populate into fields in html by ID.
        for(var x in paramsarray){  
            if  ( paramsarray[x].substring(0,4) == "ATI5" ) continue;
            if  ( paramsarray[x].substring(0,4) == "RTI5" ) continue;

            var line = paramsarray[x];
            if  (line.includes(":") && line.includes("="))
            {
                stuff = line.split(":");
                stuff2 = stuff[1].split("=");
                //x:y=z
                id=stuff[0];
                name=stuff2[0];
                value=stuff2[1];


                // javascript to make this: <tr><td>S0:FORMAT=<td><input type='text' id='S0' value='35'></tr>
                var table = document.getElementById(tableid);
                var tr = document.createElement("tr");
                var td1 = document.createElement("td");
                var td2 = document.createElement("td");
                var txt = document.createTextNode(id+":"+name);

                var mi = document.createElement("input");
                mi.setAttribute('type', 'text');
                mi.setAttribute('value', value);


                if ( fromurl == "/r900x_params_remote.txt" ) { 
                    mi.setAttribute('id', "R"+id);
                } else { 
                    mi.setAttribute('id', id);
                }

			    // make encryption fields read-only
			    if ( id == "&E" ) {
				    mi.readOnly = true;
				    mi.style.backgroundColor = "lightgray";
			    }


                td1.appendChild(txt);
                td2.appendChild(mi);

                tr.appendChild(td1);
                tr.appendChild(td2);
                table.appendChild(tr);
            }
        }

        if ( tableid == "local" ) 
        {   
            local_available = true;
            fetching_local = false;
        }
        if ( tableid == "remote") 
        { 
            remote_available = true;
            fetching_remote = false;
        }
        
        if (!fetching_local && !fetching_remote)
        {
            display_message("");
        }
    }
    if (this.readyState == 4 && this.status == 404) {

            var table = document.getElementById(tableid);
            var tr = document.createElement("tr");
            var td1 = document.createElement("td");
            var txt = document.createTextNode("Sorry, no parameters available.");

            td1.appendChild(txt);
            tr.appendChild(td1);
            table.appendChild(tr);

            if ( tableid == "local" ) { local_available = false; }
            if ( tableid == "remote") { remote_available = false; }


    }
  };
  //var url = "/r900x_params.txt"; 
  xhttp.open("GET", fromurl, true);
  xhttp.send(); 
}


function gv(id) { 
return document.getElementById(id).value; 
}
