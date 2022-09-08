import logging
import sys
logger = logging.getLogger("XPlane_Logger")
logging.basicConfig(filename='API_log.txt',
                    filemode='a',
                    level=logging.INFO,
                    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s"
)