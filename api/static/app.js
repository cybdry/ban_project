 // Function to fetch data from the server
    async function fetchData() {
      try {
        const response = await fetch('/get_data'); // Update the URL based on your actual endpoint
        const data = await response.json();
        return data;
      } catch (error) {
        console.error('Error fetching data:', error);
      }
    }
	
	function calculateLinearSpeed(x, y, z) {
      return Math.sqrt(x**2 + y**2 + z**2);
    }
    // Function to inject data into HTML elements
    async function updateData() {
      const data = await fetchData();

			console.log("Data: ",data);

      // Check if data is available
      if (data) {
        // Gyroscope data
        document.getElementById('gyroX').textContent = data.gyro.x.toFixed(3);
        document.getElementById('gyroY').textContent = data.gyro.y.toFixed(3);
        document.getElementById('gyroZ').textContent = data.gyro.z.toFixed(3);

        // Accelerometer data
        document.getElementById('accX').textContent = data.acc.x.toFixed(3);
        document.getElementById('accY').textContent = data.acc.y.toFixed(3);
        document.getElementById('accZ').textContent = data.acc.z.toFixed(3);


				const speed = calculateLinearSpeed(parseInt(data.acc.x.toFixed(3)), parseInt(data.acc.y.toFixed(3)),parseInt(data.acc.z.toFixed(3)));
				console.log("Speed :",speed)
        document.getElementById('speed').textContent = speed.toFixed(3);

        // Biometric Insights data
        document.getElementById('temp').textContent = data.temp.toFixed(2);
        document.getElementById('heart').textContent = data.heart.rate.toFixed(0);
      }
    }

    // Call the function to update data at regular intervals (e.g., every 5 seconds)
    setInterval(updateData, 5000);
