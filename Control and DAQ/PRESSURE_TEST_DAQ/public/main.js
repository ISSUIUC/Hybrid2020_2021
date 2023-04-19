
function main(){
    let ct = 1
    let lastct = 0;
    setInterval(async ()=>{
        if(lastct != ct){
            lastct = ct;
            const path = "/data"
            const data = await fetch(path);
            const json = await data.json();
            update_datafield(json);
            ct++;
        }
    }, 1000)
}

function update_datafield(data){
    const datafield = document.getElementById("DataField");
    datafield.innerHTML = "";
    for(field in data){
        let val = data[field];
        let num = parseFloat(val);
        if(!isNaN(num)) val = num;
        if( typeof val == "number") val = val.toFixed(2);

        datafield.innerHTML += `<tr><th>${field}</th><th>${val}</th></tr>`
    }
}

main();

const LaunchCodeInput = document.getElementById("LaunchCode");

document.getElementById("OpenButton").addEventListener('click', ()=>{
    const code = LaunchCodeInput.value || "nocode";
    fetch(`/open/${code}`)
})


document.getElementById("CloseButton").addEventListener('click', ()=>{
    const code = LaunchCodeInput.value || "nocode";
    fetch(`/close/${code}`)
})