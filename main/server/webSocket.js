// Разрешает или запрещает отображение элеметов редактирования.
var editMode = false;

var ip = "192.168.1.108";
//var ip = "192.168.1.102:88";

var ws = new WebSocket('ws://' + ip + "/");

ws.onopen = (msg) => {
    console.log("WebSocket open");
}

ws.onerror = (msg) => {
    console.log("WebSocket error");
    console.log(msg);
}

ws.onclose = msg => {
    console.log("WebSocket close");
};

/**
 * Обработчик входящего сообщения. Вызывается, когда от WebSocket сервера приходит сообщение, и обрабатывает его
 */
ws.onmessage = msg => {
    let commandArr = JSON.parse(msg.data);
    console.log("Input");
    console.log(commandArr);
  
    for(let i = 0; i < commandArr.length; ++i){
        let element = commandArr[i];

        //Если пришел новый контент для body
        if(element.name ==  "body"){
            document.body.innerHTML = element.value;
            document.getElementById("editMode").checked = editMode;

            if(editMode){
                setEditMode();
            }
            else{
                unsetEditMode();
            }

            console.log("New body");
            continue;
        }
        
        let elements = document.getElementsByName(element.name);
        
        if(elements.length == 0){
            console.log("Not elements with name: " + element.name);
            return;
        }

        let target = null;  
        
        for(let i = 0; i < elements.length; ++i){
            if(elements[i].closest("#" + element.homeLocation)){
                target = elements[i];
                break;
            }
        }

        if(target != null){
            switch(target.type){
                case "button":
                    buttonToggle(target, element.value);
                break;

                case "range":
                case "color":
                    target.value = element.value;
                break;

                default:
                    console.log("Not handler: " + target.type + " -> " + target.value);
            }
        }else{
            console.log("[ ERROR ] Target = null");
        }
    };
};


//TODO: Fix bug - for one object eventsHandler() may be called several time
document.addEventListener("click", eventsHandler, false);
document.addEventListener("input", eventsHandler, false);
document.addEventListener("change", eventsHandler, false);

var testArr;
var test;

var lastTarget = null;

/**
 *  Обработчик исходящего сообщения
 * @param {*} event 
 */
function eventsHandler(event)
{
    let currentTarget = event.target;

    //Чтобы для одного элемента обработчик не вызывался несколько раз, если только этот элемент не range. (ибо получаем не плавное движение и не последнее значение)
    if(lastTarget == currentTarget && currentTarget.type != "range"){
        return;
    }
    else{
        lastTarget = currentTarget;
    }

    //Спустя 200мс разрешаем любому элементу вызывать обработчик
    setTimeout(()=>{
        lastTarget = null;
    }, 200);

    //Определяем в какой локации находится элемент, вызвавший обработчик
    let homeLocation = null;
    if(currentTarget.closest("#Bedroom")){
        homeLocation = "Bedroom";
    }
    else if(currentTarget.closest("#Hall")){
        homeLocation = "Hall";
    }
    else if(currentTarget.closest("#Bathroom")){
        homeLocation = "Bathroom";
    }
    else if(currentTarget.closest("#Kitchen")){
        homeLocation = "Kitchen";
    }

    //Если объект, вызвавший обработчик, является ссылкой, то это кнопки редактирования
    if(event.target.tagName == "I" || event.target.tagName == "A"){   
        
        if(event.target.title == "edit"){
            //TODO: реализовать редактирование
            let title =  event.target.parentElement.parentElement.parentElement.children[1].textContent;
            console.log("Edit: " + homeLocation + " - " + title);
        }
        else if(event.target.title == "delete"){
            let title =  event.target.parentElement.parentElement.parentElement.children[1].textContent;
            deleteLocationElementFromDB(homeLocation, title)
        }
        else if(event.target.title == "add"){ 
            let currentTitle = getCurrentElementByName("title", homeLocation);
            let currentSelect = getCurrentElementByName("selectType", homeLocation);
            let currentPort = getCurrentElementByName("port", homeLocation);

            /**
             * Защита от неправильного ввода
             */
            if(currentTitle.value == ""){
                console.log("Title cannot be empty");
                return;
            }
            
            if(currentSelect.value == "Select type"){
                console.log("Please, select type");
                return;
            }

            if(currentPort.value == "Select port"){
                console.log("Please, select port");
                return;
            }

            addNewLocationElementToDB(homeLocation, currentTitle.value, currentSelect.value, currentPort.value);

            currentTitle.value = "Enter title";
            currentSelect.value = 0; 
            currentPort.value = 0; 

            return;
        }
    }
    
    /*
        Если элемент имеет id, то это системное сообщение, не нужно его отсылать остальным
    */
    //TODO: Переместить button к ссылка, наверх.
    if(currentTarget.id){

        if(currentTarget.type == "checkbox"){
            editMode = !editMode;

            if(editMode){
                setEditMode();
            }
            else{
                unsetEditMode();
            }

            let windowWidht = document.documentElement.clientWidth;

            if(windowWidht >= 760){
                document.getElementById("dropDownToggle").click();
            }
            else{
                document.getElementById("toggleNavBarS").click();
            }
            
            return;
        }  
    }
    else{
        /**
         * Сюда попадают элементы управления, состояние которых, следует передать всем клиентам
         */
        if(currentTarget.name == ""){
            console.log("Name not set");
            return;
        }
    
        let commandsArr = [];
        switch(currentTarget.type){
            case "button":		
                commandsArr.push({
                    "homeLocation": homeLocation,
                    "name": currentTarget.name,
                    "value": currentTarget.innerHTML == "Выключить" ? "off":"on"
                });
                break;
    
            case "range":
                commandsArr.push({
                    "homeLocation": homeLocation,
                    "name": currentTarget.name,
                    "value": currentTarget.value < 5? 0: currentTarget.value
                });
                break;
    
            case "color":
                commandsArr.push({
                    "homeLocation": homeLocation,
                    "name": currentTarget.name,
                    "value": currentTarget.value
                });
                break;
    
            case "checkbox":
                commandsArr.push({
                    "homeLocation": homeLocation,
                    "name": currentTarget.name,
                    "value": currentTarget.value
                });
                break;
   
            default:
                console.log("Not handler: id:" + event.target.id + " targetType: " + event.target.type);
                //console.log(event.target);  
                return;
        }

        commandsArrJSON = JSON.stringify(commandsArr);
        ws.send(commandsArrJSON);

        console.log("Output:");
        console.log(commandsArrJSON);
    }
}

/**
 * Возобновляет видимость элементов редактирования
 */
function setEditMode(){
    let elements = document.getElementsByName("editButtons");
    for(let i = 0; i < elements.length; ++i){
        removeClass(elements[i],"d-none");
        addClass(elements[i], "mt-n4");
        addClass(elements[i], "mr-n3");
    }

    elements = document.getElementsByName("addForm");
    for(let i = 0; i < elements.length; ++i){
        removeClass(elements[i],"d-none");
    }
}

/**
 * Отменяет видимость элементов редактирования
 */
function unsetEditMode(){
    let elements = document.getElementsByName("editButtons");
    for(let i = 0; i < elements.length; ++i){
        addClass(elements[i],"d-none");
        removeClass(elements[i], "mr-n3");
        removeClass(elements[i], "mr-n3");
    }

    elements = document.getElementsByName("addForm");
    for(let i = 0; i < elements.length; ++i){
        addClass(elements[i],"d-none");
    }
}

/**
 * Delete selected element from DB
 * @param {string} homeLocation Home location title
 * @param {string} title Title of element to delete
 */
function deleteLocationElementFromDB(homeLocation, title){
    executeAsyncRequest('http://' + ip + '/deleteLocationElement?homeLocation='+ homeLocation + '&title=' + title);
}

/**
 * Add new location element to DB
 * @param {string} homeLocation 
 * @param {string} title 
 * @param {string} type 
 * @param {string} port 
 */
function addNewLocationElementToDB(homeLocation, title, type, port){
    executeAsyncRequest('http://' + ip + '/addNewLocationElement?homeLocation='+ homeLocation + '&title=' + title + "&type=" + type + "&port=" + port);
}

/**
 * Execute request async
 * @param {string} request SQL request to DB
 */
function executeAsyncRequest(request){
    let xhr = new XMLHttpRequest();
    xhr.open('GET', request, true);
    
    xhr.send();

    xhr.onreadystatechange = function() {
        if(xhr.readyState != 4) return;

        if(xhr.status != 200){
            console.log(xhr.status + ': ' + xhr.statusText);
        }
    }
}

/**
 * Возвращает элемент с заданым именем, который находится в той же области, что и элемент вызвавший событие
 * @param {string} name Name of element to search
 * @param {string} homeLocation Title of home location
 */
function getCurrentElementByName(name, homeLocation){
    let arr = document.getElementsByName(name);
    for(let i = 0; i < arr.length; ++i){
        if(arr[i].closest("#" + homeLocation)){
            return arr[i];
        }
    }
}






/************* Визуальная часть *************/

function hasClass(elem, className) {
    return new RegExp(' ' + className + ' ').test(' ' + elem.className + ' ');
}

function addClass(elem, className) {
    if (!hasClass(elem, className)) {
        elem.className += ' ' + className;
    }
}	

function removeClass(elem, className) {
    var newClass = ' ' + elem.className.replace( /[\t\r\n]/g, ' ') + ' ';
    if (hasClass(elem, className)) {
        while (newClass.indexOf(' ' + className + ' ') >= 0 ) {
            newClass = newClass.replace(' ' + className + ' ', ' ');
        }
        elem.className = newClass.replace(/^\s+|\s+$/g, '');
    }
} 

function toggleClass(elem, className) {
    var newClass = ' ' + elem.className.replace( /[\t\r\n]/g, ' ' ) + ' ';
    if (hasClass(elem, className)) {
        while (newClass.indexOf(' ' + className + ' ') >= 0 ) {
            newClass = newClass.replace( ' ' + className + ' ' , ' ' );
        }
        elem.className = newClass.replace(/^\s+|\s+$/g, '');
    } else {
        elem.className += ' ' + className;
    }
}

function buttonToggle(button, buttonMode){
    if(buttonMode == "off"){
        removeClass(button, "btn-danger");
        addClass(button, "btn-success");
        button.innerHTML = "Включить";
    } else{
        removeClass(button, "btn-success");
        addClass(button, "btn-danger");
        button.innerHTML = "Выключить";
    }
}

