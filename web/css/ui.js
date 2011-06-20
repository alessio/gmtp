// This code has been borrowed^H^H^H^H^H^Hstolen from ArsTechnica!
// Thanks guys.

var prefsLoaded = false;
var currentFontSize = 12;
var currentFontType = 1;

function revertStyles(){
	currentFontType = 1;
	setFontFace(1);
	
	currentFontSize = 12;
	changeFontSize(0);	
}

function changeFontSize(sizeDifference){
	currentFontSize = parseInt(currentFontSize) + parseInt(sizeDifference);

	if(currentFontType == 1){
		if(currentFontSize > 16){
			currentFontSize = 16;
		}else if(currentFontSize < 8){
			currentFontSize = 8;
		}
	}else{
		if(currentFontSize > 19){
			currentFontSize = 19;
		}else if(currentFontSize < 8){
			currentFontSize = 8;
		}
	}
	setFontSize(currentFontSize);
}

function setFontSize(fontSize){
	var stObj = (document.getElementById) ? document.getElementById('ContentArea') : document.all('ContentArea');
	stObj.style.fontSize = fontSize + 'px';
}

function toggleSerif(){
	currentFontType = parseInt(currentFontType) + 1;
	if (currentFontType == 4){
		currentFontType = 1;
	}
	setFontFace(currentFontType);
}

function setFontFace(fontType){
	var stObj = (document.getElementById) ? document.getElementById('ContentArea') : document.all('ContentArea');
	switch(fontType){
		case 1: 
			stObj.style.fontFamily = 'verdana,geneva,arial,helvetica,sans-serif';
			changeFontSize(-2);
			break;
		case 2:
			stObj.style.fontFamily = 'georgia,times,times new roman,serif';
			changeFontSize(1);
			break;
		case 3:
			stObj.style.fontFamily = 'courier new,courier,serif';
			changeFontSize(1);
			break;
	}
}

function createCookie(name,value,days) {
  if (days) {
    var date = new Date();
    date.setTime(date.getTime()+(days*24*60*60*1000));
    var expires = "; expires="+date.toGMTString();
  }
  else expires = "";
  document.cookie = name+"="+value+expires+"; path=/";
}

function readCookie(name) {
  var nameEQ = name + "=";
  var ca = document.cookie.split(';');
  for(var i=0;i < ca.length;i++) {
    var c = ca[i];
    while (c.charAt(0)==' ') c = c.substring(1,c.length);
    if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
  }
  return null;
}

window.onload = setUserOptions;

function setUserOptions(){
	if(!prefsLoaded){
		cookie = readCookie("b0_fontFace");
		currentFontType = cookie ? cookie : 1;
		setFontFace(currentFontType);

		cookie = readCookie("b0_fontSize");
		currentFontSize = cookie ? cookie : 12;
		setFontSize(currentFontSize);
		
		prefsLoaded = true;
	}
}

window.onunload = saveSettings;

function saveSettings(){
  createCookie("b0_fontSize", currentFontSize, 365);
  createCookie("b0_fontFace", currentFontType, 365);
}

function SetDefaultView(){
    ShowMenu('DivHome');
    toggle('libmtp');
    toggle('libid3tag');
    toggle('libflac');
    // Get the hash after the URL, and set that area to be viewed.
    var loc = location.hash;
    if(loc != ""){
        // We have something.
        loc = 'Div' + loc.substr(1);
        ShowMenu(loc);
    }
}

function ShowMenu(div_name){
    var Menus = new Array();
    Menus[0] = 'DivHome';
    Menus[1] = 'DivRequirements';
    Menus[2] = 'DivInstallation';
    Menus[3] = 'DivDownloads';
    Menus[4] = 'DivUsage';
    Menus[5] = 'DivFAQ';
    Menus[6] = 'DivScreenshots';
    
    for (index in Menus){
        if(Menus[index] == div_name){
            show(div_name);
        } else {
            hide(Menus[index]);
        }
    }
}

function toggle(div_name) {

    var elementObject = (document.getElementById) ? document.getElementById(div_name) : document.all(div_name);
    var imageObject = (document.getElementById) ? document.getElementById(div_name + '_image') : document.all(div_name + '_image');

    if(elementObject != null){
        if(elementObject.style.display != 'none'){
            elementObject.style.display = 'none';
            if(elementObject != null){
                imageObject.src = 'plus.png';
            }
        } else {
            elementObject.style.display = 'block';
            if(elementObject != null){
                imageObject.src = 'minus.png';
            }
        }
    }
}

function hide(div_name){
    var elementObject = (document.getElementById) ? document.getElementById(div_name) : document.all(div_name);

    if(elementObject != null){
        elementObject.style.display = 'none';
    }
}

function show(div_name){
    var elementObject = (document.getElementById) ? document.getElementById(div_name) : document.all(div_name);

    if(elementObject != null){
        elementObject.style.display = 'block';
    }
}