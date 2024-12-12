// convert epochtime to JavaScripte Date object
function epochToJsDate(epochTime) {
    return new Date(epochTime);
}
// convert time to human-readable format YYYY/MM/DD HH:MM:SS
function epochToDateTime(epochTime) {
    var epochDate = new Date(epochTime);
    var dateTime = epochDate.getFullYear() + "/" +
        ("00" + (epochDate.getMonth() + 1)).slice(-2) + "/" +
        ("00" + epochDate.getDate()).slice(-2) + " " +
        ("00" + epochDate.getHours()).slice(-2) + ":" +
        ("00" + epochDate.getMinutes()).slice(-2) + ":" +
        ("00" + epochDate.getSeconds()).slice(-2);
    return dateTime;
}

// function to plot values on charts
function plotValues(chart, timestamp, value) {
    var x = epochToJsDate(timestamp).getTime();
    var y = Number(value);
    if (chart.series[0].data.length > 30) {
        chart.series[0].addPoint([x, y], true, true, true);
    } else {
        chart.series[0].addPoint([x, y], true, false, true);
    }
}

// DOM elements
const logoutNavElement = document.querySelector('#logout-link');
const loginNavElement = document.querySelector('#login-link');
const signedOutMessageElement = document.querySelector('#signedOutMessage');
const dashboardElement = document.querySelector("#dashboardSignedIn");
const userDetailsElement = document.querySelector('#user-details');
const graphOptionElement = document.querySelector('#graph-options');

//DOM elements for readings
const lastUpdateElement = document.getElementById('last-update');
const tableContainerElement = document.querySelector('#table-container');

//Buttons
const loadDataButtonElement = document.getElementById('load-data');
const viewDataButtonElement = document.getElementById('view-data-button');
const hideDataButtonElement = document.getElementById('hide-data-button');
const deleteDataElement = document.querySelector('#delete-data');


// MANAGE LOGIN/LOGOUT UI
const setupUI = (user) => {
    if (user) {
        //toggle UI elements
        logoutNavElement.style.display = 'block';
        loginNavElement.style.display = 'none';
        signedOutMessageElement.style.display = 'none';
        dashboardElement.style.display = 'block';
        graphOptionElement.style.display = 'block';
        userDetailsElement.style.display = 'block';
        userDetailsElement.innerHTML = user.email;
        // get user UID to get data from database
        var uid = user.uid;
        console.log(uid);

        // Database paths (with user UID)
        let dataAlarmPath = 'UsersData/' + uid.toString() + '/test/Alarm';
        let dataFloatPath = 'UsersData/' + uid.toString() + '/test/CH4';
        let dataIntPath = 'UsersData/' + uid.toString() + '/test/CO';
        let dataBatteryPath = 'UsersData/' + uid.toString() + '/test/VoltageBattery';
        let dataPumpPath = 'UsersData/' + uid.toString() + '/test/VoltagePump';
        let dbPathReadings = 'UsersData/' + uid.toString() + '/test';


        // Get a database reference 
        const databaseAlarm = firebase.database().ref().child(dataAlarmPath);
        const databaseCH4 = firebase.database().ref().child(dataFloatPath);
        const databaseCO = firebase.database().ref().child(dataIntPath);
        const databaseBattery = firebase.database().ref().child(dataBatteryPath);
        const databasePump = firebase.database().ref().child(dataPumpPath);

        // Variables to save database current values
        let COReadings = [];
        let CHFourReadings = [];
        let Timestamps = [];

        // Variables to save database current values
        let Alarm_Reading;
        let CH4_Reading;
        let CO_Reading;
        let batteryReading;
        let pumpReading;


        // Initialize Chart.js

        const zoomOptions = {
            zoom: {
                wheel: {
                    enabled: true,
                },
                drag: {
                    enabled: true,
                    backgroundColor: 'rgba(255, 99, 132, 0.3)', // Customize the color of the zoom rectangle
                },
                pinch: {
                    enabled: true,
                },
                pan: {
                    enabled: true,
                },
                mode: 'xy',
            },
        };


        const ctx = document.getElementById('gasChart').getContext('2d');
        const gasChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: Timestamps,
                datasets: [
                    {
                        label: 'CO Concentration (PPM)',
                        data: COReadings,
                        borderColor: 'rgba(255, 99, 132, 1)',
                        backgroundColor: 'rgba(255, 99, 132, 0.2)',
                        pointBackgroundColor: 'rgba(255, 99, 132, 1)', // Points color
                        borderDash: [5, 5], // Dashed line
                        pointRadius: 5, // Larger points
                        fill: false,
                        tension: 0.1,
                        meta: {
                            type: "co"
                        }
                    },
                    {
                        label: 'CH4 Concentration (PPM)',
                        data: CHFourReadings,
                        borderColor: 'rgba(54, 162, 235, 1)',
                        backgroundColor: 'rgba(54, 162, 235, 0.2)',
                        pointBackgroundColor: 'rgba(54, 162, 235, 1)', // Points color
                        borderDash: [], // Solid line
                        pointRadius: 7, // Larger points
                        fill: false,
                        tension: 0.1,
                        meta: {
                            type: "ch4"
                        }
                    }
                ]
            },
            options: {
                responsive: true,
                scales: {
                    x: {
                        type: 'time',
                        time: {
                            unit: 'minute',
                            tooltipFormat: 'HH:mm:ss',
                            displayFormats: {
                                second: 'HH:mm:ss'
                            }
                        },
                        title: {
                            display: true,
                            text: 'Time'
                        },
                        ticks: {
                            source: 'data',
                            autoSkip: true,
                            maxTicksLimit: 3,
                        },
                        min: new Date(Date.now() - 6 * 60 * 60 * 1000), // +1
                        max: new Date(Date.now() + 3 * 60 * 60 * 1000) // +1
                    },
                    y: {
                        title: {
                            display: true,
                            text: 'Concentration (PPM)'
                        }
                    }
                },
                plugins: {
                    zoom: zoomOptions,
                },
                onClick(e) {
                    console.log(e.type);
                }
            }
        });



        // Function to handle sending the email
        function sendAlarmEmail(message) {
            console.log("Attempting to send email..."); // Log before attempting to send email
            emailjs.send("", {      //Write your own details
                from_name: "Ship Control Management",
                message: message,
                reply_to: "email.com",  //Type your own email     
            }).then(function (response) {
                console.log('SUCCESS! : ', response.status, response.text);
            }, function (error) {
                console.log('FAILED... with error:', error);
            });

            console.log("Email send function executed."); // Log after the send function is called
        }

        //Reference to the parent node where the readings are saved
        let dbReadingsRef = firebase.database().ref(dbPathReadings);


        //for updating Alarms, Pumps, Battery, Co and CH4 values
        //databaseAlarm.on('value', (snapshot) => {
        dbReadingsRef.limitToLast(1).on('child_added', snapshot => {
            let jsonData = snapshot.toJSON();
            console.log("Json Data");
            console.log(jsonData);

            Alarm_Reading = jsonData.Alarm;
            pumpReading = jsonData.VoltagePump;
            batteryReading = jsonData.VoltageBattery;
            CO_Reading = jsonData.CO;
            CH4_Reading = jsonData.CH4;

            console.log(pumpReading);
            console.log(Alarm_Reading);
            console.log(batteryReading);
            console.log(CO_Reading);
            console.log(CH4_Reading);

            document.getElementById("reading-battery").innerHTML = batteryReading;
            document.getElementById("reading-alarm").innerHTML = Alarm_Reading;
            document.getElementById("reading-pump").innerHTML = pumpReading;
            document.getElementById("reading-CO").innerHTML = CO_Reading;
            document.getElementById("reading-CH4").innerHTML = CH4_Reading;
            if (parseInt(Alarm_Reading) == 1) {
                console.log("Upozorenje za Pumpu");
                sendAlarmEmail("UPOZORENJE! Pumpa radi već preko 15 minuta");
            }
            if (parseInt(batteryReading) < 9 && parseInt(batteryReading) > 1) {
                console.log("Upozorenje za bateriju");
                sendAlarmEmail("UPOZORENJE! Napon pumpe je ispod 9V");
            }
            if (parseFloat(CO_Reading) > 400) {
                console.log("Upozorenje za visoku razinu ugljikovog dioksida");
                sendAlarmEmail("UPOZORENJE! Visoka razina CO");
            }
            if (parseFloat(CH4_Reading) > 50000) {
                console.log("Upozorenje za visoku razinu metana");
                sendAlarmEmail("UPOZORENJE! Visoka razina CH4");
            }

        });

        //ChartJS graph
        dbReadingsRef.orderByKey().limitToLast(50).on('child_added', snapshot => {
            var jsonData = snapshot.toJSON(); // example: {temperature: 25.02, humidity: 50.20, pressure: 1008.48, timestamp:1641317355}
            //console.log(jsonData);
            // Save values on variables
            let co = jsonData.CO;
            let ch4 = jsonData.CH4;
            let timestamp = jsonData.timestamp;

            var x = epochToJsDate(timestamp).getTime();
            Timestamps.push(x);
            //ch4Timestamps.push(x);
            gasChart.data.labels = Timestamps;
            gasChart.data.datasets.forEach((dataset) => {
                //provjeri koji je dataset, pushaj u odgovarajuci
                if (dataset.meta.type == "co") {
                    console.log("dataset CO");
                    console.log(dataset);
                    dataset.data.push(co);
                } else if (dataset.meta.type == "ch4") {
                    console.log("dataset CH4");
                    console.log(dataset);
                    dataset.data.push(ch4);
                }
                // Check if the dataset has more than 30 points
                gasChart.data.datasets.forEach((dataset) => {
                    if (dataset.data.length > 40) {
                        // Remove the first data point and label to maintain 30 points
                        dataset.data.shift();
                        gasChart.data.labels.shift();
                    }
                });
                gasChart.update();
            });
        });

        //Reset Button
        document.getElementById('resetZoomButton').addEventListener('click', function () {
            gasChart.resetZoom();
        });


        //for updating Last Update
        dbReadingsRef.limitToLast(1).on('child_added', snapshot => {
            let jsonData = snapshot.toJSON();
            let timestamp = jsonData.timestamp;

            lastUpdateElement.innerHTML = epochToDateTime(timestamp);
        });


        // Render charts to display data
        chartA = createAlarmChart();
        chartB = createBatteryChart();
        chartP = createPumpChart();
        chartCO = createCOChart();
        chartCH = createCH4Chart();

        //Get the latest 30 readings to display on charts
        dbReadingsRef.orderByKey().limitToLast(50).on('child_added', snapshot => {
            var jsonData = snapshot.toJSON(); // example: {temperature: 25.02, humidity: 50.20, pressure: 1008.48, timestamp:1641317355}
            //console.log(jsonData);
            // Save values on variables
            let alarm = jsonData.Alarm;
            let pump = jsonData.VoltagePump;
            let battery = jsonData.VoltageBattery;
            let co = jsonData.CO;
            let ch4 = jsonData.CH4;
            let timestamp = jsonData.timestamp;
            // Plot the values on charts
            plotValues(chartA, timestamp, alarm);
            plotValues(chartB, timestamp, battery);
            plotValues(chartP, timestamp, pump);
            plotValues(chartCO, timestamp, co);
            plotValues(chartCH, timestamp, ch4);
        });

        let lastID; //saves the pushID of the last readings displayed on the table
        function createTable() {
            // Append all data to the table
            var firstRun = true;
            dbReadingsRef.orderByKey().limitToLast(50).on('child_added', function (snapshot) {
                if (snapshot.exists()) {
                    let jsonData = snapshot.toJSON();
                    //console.log(jsonData);
                    let Alarm = jsonData.Alarm;
                    let battery = jsonData.VoltageBattery;
                    let pump = jsonData.VoltagePump;
                    let co = jsonData.CO;
                    let ch4 = jsonData.CH4;
                    let timestamp = jsonData.timestamp;

                    let content = '';
                    content += '<tr>';
                    content += '<td>' + epochToDateTime(timestamp) + '</td>';
                    content += '<td>' + battery + '</td>';
                    content += '<td>' + pump + '</td>';
                    content += '<td>' + Alarm + '</td>';
                    content += '<td>' + co + '</td>';
                    content += '<td>' + ch4 + '</td>';
                    content += '</tr>';

                    $('#table-body').prepend(content);
                    // Save lastID --> corresponds to the first key on the returned snapshot data
                    if (firstRun) {
                        lastID = snapshot.key;
                        firstRun = false;
                        //console.log(lastID);
                    }
                }
            });
        }

        // Button that shows the table
        viewDataButtonElement.addEventListener('click', (e) => {
            // Toggle DOM elements
            tableContainerElement.style.display = 'block';
            viewDataButtonElement.style.display = 'none';
            hideDataButtonElement.style.display = 'inline-block';
            loadDataButtonElement.style.display = 'inline-block'
            createTable();
        });

        // Append readings to table (after pressing More results... button)
        function appendToTable() {
            var dataList = []; // saves list of readings returned by the snapshot (oldest-->newest)
            var reversedList = []; // the same as previous, but reversed (newest--> oldest)
            console.log("APEND");
            dbReadingsRef.orderByKey().limitToLast(50).endAt(String(lastID)).once('value', function (snapshot) {

                if (snapshot.exists()) {
                    var firstRun = true;
                    snapshot.forEach(element => {
                        // Get the last pushID (it's the first on the snapshot oldest data --> newest data)
                        if (firstRun) {
                            lastID = element.key
                            //console.log("last reading");
                            //console.log(lastID);
                            firstRun = false;
                        }
                        var jsonData = element.toJSON();
                        //console.log(jsonData);
                        dataList.push(jsonData); // create a list with all data
                        //console.log(dataList);
                    });
                    reversedList = dataList.reverse(); // reverse the order of the list (newest data --> oldest data)
                    var firstTime = true;
                    // loop through all elements of the list and append to table (newest elements first)
                    reversedList.forEach(element => {
                        if (firstTime) { // ignore first reading (it's already on the table from the previous query)
                            firstTime = false;
                        } else { //append the new readings to the table
                            let Alarm = element.Alarm;
                            let battery = element.VoltageBattery;
                            let pump = element.VoltagePump;
                            let co = element.CO;
                            let ch4 = element.CH4;
                            let timestamp = element.timestamp;
                            let content = '';
                            content += '<tr>';
                            content += '<td>' + epochToDateTime(timestamp) + '</td>';
                            content += '<td>' + battery + '</td>';
                            content += '<td>' + pump + '</td>';
                            content += '<td>' + Alarm + '</td>';
                            content += '<td>' + co + '</td>';
                            content += '<td>' + ch4 + '</td>';
                            content += '</tr>';

                            $('#table-body').append(content);
                        }

                    });
                }
            });
        }
        // When you click on the more results... button, call the appendToTable function
        loadDataButtonElement.addEventListener('click', (e) => {
            appendToTable();
        });

        // Hide the table when you click on Hide Data
        hideDataButtonElement.addEventListener('click', (e) => {
            tableContainerElement.style.display = 'none';
            viewDataButtonElement.style.display = 'inline-block';
            hideDataButtonElement.style.display = 'none';
        });

        deleteDataElement.addEventListener('click', (e) => {
            // delete data (readings)
            dbReadingsRef.remove();
            window.location.reload();
        });

        // if user is logged out
    } else {
        // toggle UI elements
        logoutNavElement.style.display = 'none';
        loginNavElement.style.display = 'block';
        signedOutMessageElement.style.display = 'block';
        dashboardElement.style.display = 'none';
        graphOptionElement.style.display = 'none';
        userDetailsElement.style.display = 'none';

    }
}


