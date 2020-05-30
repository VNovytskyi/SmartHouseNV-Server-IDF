
var homeLocations;
var elementTypes;
var locationElements;


var xhr = new XMLHttpRequest();
xhr.open('GET', 'http://192.168.1.108/getHomeLocations', false);
xhr.send();

if (xhr.status != 200) {
    console.log( xhr.status + ': ' + xhr.statusText );
} 
else {
    homeLocations = JSON.parse(xhr.responseText);
    let navBar = document.getElementById('navBar');
    
    
    let buff = '';
    for(let i = 0; i < homeLocations.length; ++i){
         buff += `<li class='nav-item active'><a class='nav-link' href='#` + homeLocations[i].title + `'>` + homeLocations[i].title + `<span class='sr-only'></span></a></li>` ;
    }

    navBar.innerHTML = buff + navBar.innerHTML;

    for(let i = 0; i < homeLocations.length; ++i){
        if(i % 2 == 0){
            document.body.innerHTML +=  `<div id='` + homeLocations[i].title + `' class='min-vh-100'>` + 
                                            `<h1 class='text-center font-italic pt-5'>` + homeLocations[i].title + `</h1>
                                             <div class='container mt-4'>
                                                <div class='row'>
                                                </div>
                                             </div>
                                         </div>`;
        }
        else{
            document.body.innerHTML +=  `<div id='` + homeLocations[i].title + `' class='min-vh-100 bg-light'>` + 
                                            `<h1 class='text-center font-italic pt-5'>` + homeLocations[i].title + `</h1>
                                                <div class='container mt-4'>
                                                    <div class='row'>
                                                    </div>
                                                </div>
                                         </div>`;
        }
    }
}

xhr.open('GET', 'http://192.168.1.108/getElementTypes', false);
xhr.send();

if (xhr.status != 200) {
    console.log( xhr.status + ': ' + xhr.statusText );
} 
else {
    elementTypes = JSON.parse(xhr.responseText);
}


xhr.open('GET', 'http://192.168.1.108/getLocationElements', false);
xhr.send();

if (xhr.status != 200) {
    console.log( xhr.status + ': ' + xhr.statusText );
} 
else {
    locationElements = JSON.parse(xhr.responseText);
    
    
    for(let i = 0; i < homeLocations.length; ++i){

        let target = document.getElementById('Bedroom').firstElementChild.nextElementSibling.firstElementChild;

        for(let j = 0; j < locationElements.length; ++j)
        {
            if(locationElements[j].idHomeLocation == homeLocations[i].idHomeLocation)
            {
                let idElement = locationElements[j].idElementType;
                let elementTitle = elementTypes[idElement - 1].title;
                let locationTitle = locationElements[j].title;

    
                
                let colmd = 'col-md-3';
                let buff = '';

                switch(elementTitle)
                {
                    case 'SimpleButton':
                        buff += `<button type='button' class='btn btn-outline-success'>Click</button>`;
                        break;
                    
                    case 'DoubleButton':
                        buff += `<div class='btn-group'>
                                    <button type='button' class='btn btn-outline-success'>On</button>
                                    <button type='button' class='btn btn-outline-danger'>Off</button>
                                </div>`;
                        break;

                    case 'TripleButton':
                        buff += `<div class='btn-group'>
                                    <button type='button' class='btn btn-outline-success'>Open</button>
                                    <button type='button' class='btn btn-outline-warning'>Stop</button>
                                    <button type='button' class='btn btn-outline-danger'>Close</button>
                                </div>`;
                        break;

                    case 'ChangingButton':
                        buff += `<button name='` + locationElements[j].port + `' type='button' class='btn btn-success'>Включить</button>`;
                        break;
                            
                    case 'Range':
                        colmd = 'col-md-6';
                        buff += `<input name='` + locationElements[j].port + `' type='range' min='1' max='100' value='0' class='slider'>`;
                        break;
                    
                    case 'ColorPicker':
                        buff += `<input name='` + locationTitle + `' type='color' value='#fd3cf8'>
                                <div class='btn-group'>
                                    <button type='button' class='btn btn-outline-success'>On</button>
                                    <button type='button' class='btn btn-outline-danger'>Off</button>
                                </div>`;
                        break;

                    default:
                        console.log('Element type error');
                    break;
                }
                
                buff = `<div class='` + colmd + `mb-4'>
                            <div class='card'>
                                <div class='card-body mt-1'>
                                <div name='editButtons' class='text-right d-none'>
                                    <button type='button' style='border: none;background: inherit;'>
                                        <i title='delete' class='fas fa-times-circle text-danger'></i>
                                    </button>
                                </div>
                                    <h5 class='card-title'>` + locationTitle + `</h5>` + buff;
                
                target.innerHTML += buff;
                
                
                target.innerHTML += `</div></div></div>`;
            }
        }
        target.innerHTML += `</div></div></div>`;
    }
}